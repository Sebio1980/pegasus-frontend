// Pegasus Frontend
// Copyright (C) 2017-2018  Mátyás Mustoha
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


#include "Log.h"

#include "AppSettings.h"
#include "Paths.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <iostream>

//For recalbox
#include "RecalboxConf.h"
#include "RootFolders.h"
#include <utils/os/fs/Path.h>

#if defined(Q_OS_ANDROID) && defined(QT_DEBUG)
#include <android/log.h>
#endif // defined(Q_OS_ANDROID) && defined(QT_DEBUG)


LogSink::LogSink() = default;
LogSink::~LogSink() = default;


namespace logsinks {

class QtLog : public LogSink {
public:
    void debug(const QString& msg) override {
        if (RecalboxConf::Instance().AsBool("pegasus.debuglogs")) qDebug().noquote().nospace() << msg;
    }
    void info(const QString& msg) override {
        qInfo().noquote().nospace() << msg;
    }
    void warning(const QString& msg) override {
        qWarning().noquote().nospace() << msg;
    }
    void error(const QString& msg) override {
        qWarning().noquote().nospace() << msg;
    }
};


class Terminal : public LogSink {
public:
    Terminal()
        : m_stream(stdout)
    {}
    void debug(const QString& msg) override {
        if (RecalboxConf::Instance().AsBool("pegasus.debuglogs")) colorlog(m_pre_debug, m_marker_debug, msg);
    }
    void info(const QString& msg) override {
        colorlog(m_pre_info, m_marker_info, msg);
    }
    void warning(const QString& msg) override {
        colorlog(m_pre_warning, m_marker_info, msg);
    }
    void error(const QString& msg) override {
        colorlog(m_pre_error, m_marker_error, msg);
    }

private:
    QTextStream m_stream;

#ifdef Q_OS_WIN
    static constexpr auto m_pre_debug = "[d]";
    static constexpr auto m_pre_info = "[i]";
    static constexpr auto m_pre_warning = "[w]";
    static constexpr auto m_pre_error = "[e]";
    static constexpr auto m_fmt_reset = "";
#else
    static constexpr auto m_pre_debug = "\x1b[34m";
    static constexpr auto m_marker_debug = "[d]";
    static constexpr auto m_pre_info = "\x1b[0m";
    static constexpr auto m_marker_info = "[i]";
    static constexpr auto m_pre_warning = "\x1b[93m";
    static constexpr auto m_marker_warning = "[w]";
    static constexpr auto m_pre_error = "\x1b[91m";
    static constexpr auto m_marker_error = "[e]";
    //static constexpr auto m_fmt_reset = "\x1b[0m";
#endif

    void colorlog(const char* const prefix, const char* const marker, const QString& msg) {
        try{
            //crash identified just after 06/03/20224
            //try/catch added to avoid it
            QDateTime dateTime = QDateTime::currentDateTime();
            if(dateTime.isValid()){
                QString isoDate = dateTime.toString(Qt::ISODate);
                m_stream << prefix << QChar(' ') << isoDate << QChar(' ')
                         << marker << QChar(' ') << msg << Qt::endl;
            }
        } catch ( const std::exception & Exp )
        {
            std::cout << "Exception catched : " << Exp.what() << std::endl;
        }
    }
};


class LogFile : public LogSink {
public:
    LogFile()
        : m_file(default_log_path())
    {
        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            Log::warning(LOGMSG("Could not open `%1` for writing, file logging disabled.")
                         .arg(m_file.fileName()));
            return;
        }

        m_stream.setDevice(&m_file);
    }

    void debug(const QString& msg) override {
        if (Q_UNLIKELY(!m_file.isOpen()))
            return;

        if (RecalboxConf::Instance().AsBool("pegasus.debuglogs")) datelog(m_marker_debug, msg);
    }
    void info(const QString& msg) override {
        if (Q_UNLIKELY(!m_file.isOpen()))
            return;

        datelog(m_marker_info, msg);
    }
    void warning(const QString& msg) override {
        if (Q_UNLIKELY(!m_file.isOpen()))
            return;

        datelog(m_marker_warning, msg);
        m_stream.flush();
    }
    void error(const QString& msg) override {
        if (Q_UNLIKELY(!m_file.isOpen()))
            return;

        datelog(m_marker_error, msg);
        m_stream.flush();
    }

private:
    QFile m_file;
    QTextStream m_stream;

    static constexpr auto m_marker_debug = "[d]";
    static constexpr auto m_marker_info = "[i]";
    static constexpr auto m_marker_warning = "[w]";
    static constexpr auto m_marker_error = "[e]";

    QString default_log_path() {
        Path folder = RootFolders::DataRootFolder / "system/logs" / "lastrun.log";
        return QString::fromStdString(folder.ToString());
    }

    void datelog(const char* const marker, const QString& msg) {
        try{
            //crash identified here 10//03/2024 03h49
            //try/catch added to avoid it
            QDateTime dateTime = QDateTime::currentDateTime();
            if(dateTime.isValid()){
                QString isoDate = dateTime.toString(Qt::ISODate);
                m_stream << isoDate << QChar(' ')
                     << marker << QChar(' ')
                     << msg << QChar('\n');
                }
        } catch ( const std::exception & Exp )
        {
            std::cout << "Exception catched : " << Exp.what() << std::endl;
        }
    }
};

#if defined(Q_OS_ANDROID) && defined(QT_DEBUG)
class AndroidLogcat : public LogSink {
public:
    AndroidLogcat() {}

    void info(const QString& msg) override {
        write_log(ANDROID_LOG_DEBUG, m_marker_info, msg);
    }
    void warning(const QString& msg) override {
        write_log(ANDROID_LOG_WARN, m_marker_warning, msg);
    }
    void error(const QString& msg) override {
        write_log(ANDROID_LOG_ERROR, m_marker_error, msg);
    }

private:
    static constexpr auto m_appname = "pegasus-fe";
    static constexpr auto m_marker_info = "[i] ";
    static constexpr auto m_marker_warning = "[w] ";
    static constexpr auto m_marker_error = "[e] ";

    void write_log(const int prio, const char* const prefix, const QString& msg) {
        const QString out = QLatin1String(prefix) + msg;
        __android_log_write(prio, m_appname, out.toLocal8Bit().constData());
    }
};
#endif // defined(Q_OS_ANDROID) && defined(QT_DEBUG)
} // namespace logsinks


namespace {

void on_qt_message(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    const QString prepared_msg = qFormatLogMessage(type, context, msg);
    switch (type) {
        case QtMsgType::QtDebugMsg:
            if (RecalboxConf::Instance().AsBool("pegasus.debuglogs")) Log::debug(prepared_msg);
            break;
        case QtMsgType::QtInfoMsg:
            Log::info(prepared_msg);
            break;
        case QtMsgType::QtWarningMsg:
            Log::warning(prepared_msg);
            break;
        case QtMsgType::QtCriticalMsg:
        case QtMsgType::QtFatalMsg:
            Log::error(prepared_msg);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

} // namespace


std::vector<std::unique_ptr<LogSink>> Log::m_sinks {};

void Log::init(bool silent)
{
    if (!silent) {
        m_sinks.emplace_back(new logsinks::Terminal());
        #if defined(Q_OS_ANDROID) && defined(QT_DEBUG)
        m_sinks.emplace_back(new logsinks::AndroidLogcat);
        #endif // defined(Q_OS_ANDROID) && defined(QT_DEBUG)
    }

    m_sinks.emplace_back(new logsinks::LogFile());

    // redirect Qt messages to the Log too
    qInstallMessageHandler(on_qt_message);
}

void Log::init_qttest()
{
    // QtTests only notice messages made through QDebug
    m_sinks.emplace_back(new logsinks::QtLog());
}

void Log::close()
{
    m_sinks.clear();
}

#define FORALLSINK_CALLER(method) \
    void Log::method(const QString& message) \
    { \
        for (const auto& sink : m_sinks) \
            sink->method(message); \
    } \
    void Log::method(const QString& tag, const QString& message) \
    { \
        const QString combi_msg = QStringLiteral("%1: %2").arg(tag, message); \
        Log::method(combi_msg); \
    }
FORALLSINK_CALLER(debug)
FORALLSINK_CALLER(info)
FORALLSINK_CALLER(warning)
FORALLSINK_CALLER(error)
