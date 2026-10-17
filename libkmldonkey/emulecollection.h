/* -*- c++ -*-
 *
 * emulecollection.h
 *
 * Copyright (C) 2007  Johannes Krampf <wuischke@amule.org>
 * Copyright (C) 2008  Gioacchino Mazzurco <gmazzurco89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __libkmldonkey_EMULECOLLECTION_H__
#define __libkmldonkey_EMULECOLLECTION_H__

#include <string>
#include <vector>
#include <stdint.h>

class EmuleCollection{
	public:
		bool Open( std::string File );
		size_t GetFileCount();
							// Return values on error		Return value no error
		std::string GetEd2kLink(unsigned int index);	// "Invalid Index"			valid ed2k link
		std::string GetFileName(unsigned int index);	// "Empty String", "Invalid Index"	valid file name
		uint64_t GetFileSize(unsigned int index);	// 0					valid file size
		std::string GetFileHash(unsigned int index);	// "Empty String", "Invalid Index"	valid file hash
	private:
		bool OpenBinary( std::string File );
		bool OpenText( std::string File );
		bool AddLink( std::string Link );
		bool AddFile( std::string FileName, uint64_t FileSize, std::string FileHash);
		bool IsValidHash( std::string FileHash );
		struct CollectionFile{
			std::string FileName;
			uint64_t FileSize;
			std::string FileHash; 
		};
		std::vector<CollectionFile> vCollection;	
};

#endif
