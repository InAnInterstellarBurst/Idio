/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "idio.hpp"

int main(int argc, char** argv)
{
	Idio::CommandArgs args(static_cast<size_t>(argc));
	args.insert(args.end(), &argv[0], &argv[argc - 1]);
	Idio::main(args);
	return EXIT_SUCCESS;
}