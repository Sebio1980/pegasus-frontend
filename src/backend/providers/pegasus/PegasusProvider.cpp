// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "PegasusProvider.h"

#include "ConfigFile.h"
#include "Utils.h"
#include "types/Collection.h"
#include "types/Game.h"
#include "types/Collection.h"

#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>


namespace {

QStringList load_game_dir_list()
{
    constexpr int LINE_MAX_LEN = 4096;

    QStringList rom_dirs;
    for (QString& path : ::configDirPaths()) {
        path += QStringLiteral("/game_dirs.txt");

        QFile config_file(path);
        if (!config_file.open(QFile::ReadOnly | QFile::Text))
            continue;

        qInfo() << QObject::tr("Found `%1`").arg(path);

        QTextStream stream(&config_file);
        QString line;
        while (stream.readLineInto(&line, LINE_MAX_LEN)) {
            if (!line.startsWith('#'))
                rom_dirs << line;
        }
    }

    return rom_dirs;
}

enum class AttribType : unsigned char {
    NAME,
    LAUNCH_CMD,
    EXTENSIONS,
    FILES,
    REGEX,
};
struct GameFilterGroup {
    QStringList extensions;
    QStringList files;
    QString regex;
};
struct GameFilter {
    GameFilterGroup include;
    GameFilterGroup exclude;
    QStringList extra;
};

QHash<QString, GameFilter> read_collections_file(const QString& dir_path,
                                                 QHash<QString, Types::Collection*>& collections)
{
    // reminder: sections are collection tags
    // including keys: extensions, files, regex
    // excluding keys: ignore-extensions, ignore-files, ignore-regex
    // optional: name, launch

    static const QHash<QString, AttribType> key_types {
        { QStringLiteral("name"), AttribType::NAME },
        { QStringLiteral("launch"), AttribType::LAUNCH_CMD },
        { QStringLiteral("extension"), AttribType::EXTENSIONS },
        { QStringLiteral("extensions"), AttribType::EXTENSIONS },
        { QStringLiteral("file"), AttribType::FILES },
        { QStringLiteral("files"), AttribType::FILES },
        { QStringLiteral("regex"), AttribType::REGEX },
        { QStringLiteral("ignore-extension"), AttribType::EXTENSIONS },
        { QStringLiteral("ignore-extensions"), AttribType::EXTENSIONS },
        { QStringLiteral("ignore-file"), AttribType::FILES },
        { QStringLiteral("ignore-files"), AttribType::FILES },
        { QStringLiteral("ignore-regex"), AttribType::REGEX },
    };

    QString curr_file_path;
    QString curr_coll_name;
    QHash<QString, GameFilter> config;

    const auto on_section = [&curr_coll_name, &collections, &dir_path](const int, const QString name){
        curr_coll_name = name;

        if (!collections.contains(name))
            collections.insert(name, new Types::Collection(name));

        collections[name]->sourceDirsMut().append(dir_path);
    };
    const auto on_attribute = [&](const int lineno, const QString key, const QString val){
        if (curr_coll_name.isEmpty()) {
            qWarning().noquote()
                << QObject::tr("`%1`, line %2: no sections defined yet, values ignored")
                               .arg(curr_file_path, QString::number(lineno));
            return;
        }

        GameFilter& filter = config[curr_coll_name];
        if (key.startsWith(QLatin1String("x-"))) {
            // TODO: unimplemented
            return;
        }
        if (!key_types.contains(key)) {
            qWarning().noquote()
                << QObject::tr("`%1`, line %2: unrecognized attribute name `%3`, ignored")
                               .arg(curr_file_path, QString::number(lineno), key);
            return;
        }

        GameFilterGroup& filter_group = key.startsWith(QLatin1String("ignore-"))
            ? filter.exclude
            : filter.include;
        switch (key_types[key]) {
            case AttribType::NAME:
                collections[curr_coll_name]->setName(val);
                break;
            case AttribType::LAUNCH_CMD:
                collections[curr_coll_name]->setCommonLaunchCmd(val);
                break;
            case AttribType::EXTENSIONS:
                filter_group.extensions.append(::tokenize(val.toLower()));
                break;
            case AttribType::FILES:
                filter_group.files.append(::tokenize(val));
                break;
            case AttribType::REGEX:
                if (!filter_group.regex.isEmpty()) {
                    qWarning().noquote()
                        << QObject::tr("`%1`, line %2: `%3` was already defined for this collection, replaced")
                                       .arg(curr_file_path, QString::number(lineno), key);
                }
                filter_group.regex = val;
                break;
        }

    };
    const auto on_error = [&](const int lineno, const QString msg){
        qWarning().noquote()
            << QObject::tr("`%1`, line %2: %3")
                           .arg(curr_file_path, QString::number(lineno), msg);
    };


    // the actual reading

    curr_file_path = dir_path + QStringLiteral("/collections.pegasus.txt");
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    curr_file_path = dir_path + QStringLiteral("/collections.txt");
    curr_coll_name.clear();
    config::readFile(curr_file_path, on_section, on_attribute, on_error);

    // cleanup and return

    auto config_it = config.begin();
    for (; config_it != config.end(); ++config_it) {
        GameFilter& filter = config_it.value();
        filter.include.extensions.removeDuplicates();
        filter.include.files.removeDuplicates();
        filter.exclude.extensions.removeDuplicates();
        filter.exclude.files.removeDuplicates();
        filter.extra.removeDuplicates();
    }
    return config;
}

void traverse_dir(const QString& dir_base_path,
                  const QHash<QString, GameFilter>& filter_config,
                  const QHash<QString, Types::Collection*>& collections,
                  QHash<QString, Types::Game*>& games)
{
    constexpr auto entry_filters = QDir::Files | QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
    constexpr auto entry_flags = QDirIterator::FollowSymlinks;

    auto config_it = filter_config.constBegin();
    for (; config_it != filter_config.constEnd(); ++config_it) {
        const GameFilter& filter = config_it.value();
        const QRegularExpression include_regex(filter.include.regex);
        const QRegularExpression exclude_regex(filter.exclude.regex);

        // find all dirs and subdirectories, but ignore 'media'

        QStringList subdirs;
        {
            constexpr auto subdir_filters = QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot;
            constexpr auto subdir_flags = QDirIterator::FollowSymlinks | QDirIterator::Subdirectories;

            QDirIterator dirs_it(dir_base_path, subdir_filters, subdir_flags);
            while (dirs_it.hasNext()) {
                subdirs << dirs_it.next();
            }
            subdirs.removeOne(dir_base_path + QStringLiteral("/media"));
            subdirs.append(dir_base_path + QStringLiteral("/")); // added "/" so all entries have base + 1 length
        }

        // run through the main dir and all valid subdirs

        for (const QString& dir : qAsConst(subdirs)) {
            QDirIterator dir_it(dir, entry_filters, entry_flags);
            while (dir_it.hasNext()) {
                dir_it.next();
                const QFileInfo fileinfo = dir_it.fileInfo();
                const QString relative_path = fileinfo.filePath().mid(dir_base_path.length() + 1);

                const bool exclude = filter.exclude.extensions.contains(fileinfo.suffix())
                    || filter.exclude.files.contains(relative_path)
                    || (!filter.exclude.regex.isEmpty() && exclude_regex.match(fileinfo.filePath()).hasMatch());
                if (exclude)
                    continue;

                const bool include = filter.include.extensions.contains(fileinfo.suffix())
                    || filter.include.files.contains(relative_path)
                    || (!filter.include.regex.isEmpty() && include_regex.match(fileinfo.filePath()).hasMatch());
                if (!include)
                    continue;

                Types::Collection* const& collection_ptr = collections[config_it.key()];
                Types::Game*& game_ptr = games[fileinfo.canonicalFilePath()];
                if (!game_ptr)
                    game_ptr = new Types::Game(fileinfo, collection_ptr);

                collection_ptr->gameListMut().addGame(game_ptr);
            }
        }
    }
}

} // namespace


namespace providers {
namespace pegasus {

PegasusProvider::PegasusProvider(QObject* parent)
    : Provider(parent)
    , m_game_dirs(load_game_dir_list())
{}

void PegasusProvider::find(QHash<QString, Types::Game*>& games,
                           QHash<QString, Types::Collection*>& collections)
{
    find_in_dirs(m_game_dirs, games, collections);
}

void PegasusProvider::enhance(const QHash<QString, Types::Game*>&,
                              const QHash<QString, Types::Collection*>&)
{}

void PegasusProvider::find_in_dirs(const QStringList& dir_list,
                                   QHash<QString, Types::Game*>& games,
                                   QHash<QString, Types::Collection*>& collections)
{
    for (const QString& dir_path : dir_list) {
        const auto filter_config = read_collections_file(dir_path, collections);
        traverse_dir(dir_path, filter_config, collections, games);
    }
}

} // namespace pegasus
} // namespace providers
