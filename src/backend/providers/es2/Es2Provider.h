// Pegasus Frontend
// Copyright (C) 2017-2020  Mátyás Mustoha
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
//
// Updated and integrated for recalbox by BozoTheGeek 03/05/2021
//

#pragma once

#include "providers/Provider.h"
#include "providers/es2/Es2Input.h"
#include "providers/es2/Es2Systems.h"

namespace providers {
namespace es2 {

class Es2Provider : public Provider {
    Q_OBJECT

public:
    explicit Es2Provider(QObject* parent = nullptr);

    Provider& run(SearchContext&) final;
    inputConfigEntry load_input_data(const QString&, const QString&);
    inputConfigEntry load_any_input_data_by_guid(const QString&);
    bool save_input_data(const inputConfigEntry&);
	SystemEntry find_one_system(const QString shortName);
};

} // namespace es2
} // namespace providers
