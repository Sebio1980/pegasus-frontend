// Pegasus Frontend
// Copyright (C) 2018  Mátyás Mustoha
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


#pragma once

class QString;
class QStringList;


namespace paths {

/// Returns $PEGASUS_HOME if defined, or $HOME if defined,
/// otherwise QDir::homePath().
QString homePath();

/// Returns the directory paths where config files may be located
QStringList configDirs();

/// Returns the directory paths where themes may be located
QStringList themesDirs(bool forcedRefresh = false);

/// Returns the directory paths where roms could be located
QStringList romsDirs(bool forcedRefresh = false);

/// Returns the directory paths where saves may be located
QStringList savesDirs(bool forcedRefresh = false);

/// Returns the directory paths where bios may be located
QStringList biosDirs(bool forcedRefresh = false);

/// Returns the directory paths where music may be located
QStringList musicDirs(bool forcedRefresh = false);

/// Returns the directory paths where bios overlays be located
QStringList overlaysDirs(bool forcedRefresh = false);

/// Returns the directory paths where screenshots may be located
QStringList screenshotsDirs(bool forcedRefresh = false);

/// Returns the directory paths where shaders may be located
QStringList shadersDirs(bool forcedRefresh = false);

/// Returns the directory paths where userscripts may be located
QStringList userscriptsDirs(bool forcedRefresh = false);

/// Returns the directory paths where videos may be located
QStringList videosDirs(bool forcedRefresh = false);

/// Returns a directory path where persistent data could be stored
QString writableConfigDir();

/// Returns a directory path where cache data could be stored
QString writableCacheDir();

} // namespace paths
