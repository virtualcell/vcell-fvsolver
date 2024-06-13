/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
/////////////////////////////////////////////////////////////
// ZipUtils.h
///////////////////////////////////////////////////////////
#ifndef ZIPUTILS_H
#define ZIPUTILS_H

#include <filesystem>

extern void extractFileFromZip(std::filesystem::path zipFilename, std::filesystem::path zipEntryName);
extern void addFilesToZip(std::filesystem::path zipFilename, std::filesystem::path filename1, std::filesystem::path filename2="");

#endif
