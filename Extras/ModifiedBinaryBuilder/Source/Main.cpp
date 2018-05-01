/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/*
  ==============================================================================

   Utility to turn a bunch of binary files into a .cpp file and .h file full of
   data so they can be built directly into an executable.

   Use this code at your own risk! It carries no warranty!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
static int addFile (const File& file,
                    const String& classname,
                    OutputStream& headerStream,
                    OutputStream& cppStream)
{
    MemoryBlock mb;
    file.loadFileAsData (mb);
    const String name (file.getFileName()
                           .replaceCharacter (' ', '_')
                           .replaceCharacter ('.', '_')
                           .retainCharacters ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_0123456789"));

    std::cout << "Adding " << name << ": "
              << (int) mb.getSize() << " bytes" << std::endl;

    //headerStream << "    double [301][512]  " << name << ";\r\n";
                    //"    const int           " << name << "Size = "
                 //<< (int) 512 << ";\r\n\r\n";
    
    int iOnes = name.toStdString()[10] - '0';
    int iTens = name.toStdString()[9] - '0';
    int iHundreds = name.toStdString()[7] - '0';
    
    cppStream << "{";

    StringArray temp;
    file.readLines(temp);
    //std::cout << temp[0] << std::endl;
    if (name.toStdString()!= "TriSaw_1_00_txt")
    {
        cppStream << temp[0] << "},\r\n";
    }
    else
    {
        cppStream << temp[0] << "}\r\n";
    }
    

    //cppStream << "const double* " << classname << "::" << name
              //<< " = (const double*) temp" << tempNum << ";\r\n\r\n";

    return (int) mb.getSize();
}

static bool isHiddenFile (const File& f, const File& root)
{
    return f.getFileName().endsWithIgnoreCase (".scc")
         || f.getFileName() == ".svn"
         || f.getFileName().startsWithChar ('.')
         || (f.getSize() == 0 && ! f.isDirectory())
         || (f.getParentDirectory() != root && isHiddenFile (f.getParentDirectory(), root));
}


//==============================================================================
int main (int argc, char* argv[])
{
    std::cout << std::endl << " BinaryBuilder!  Visit www.juce.com for more info." << std::endl;

    if (argc < 4 || argc > 5)
    {
        std::cout << " Usage: BinaryBuilder  sourcedirectory targetdirectory targetclassname [optional wildcard pattern]\n\n"
                     " BinaryBuilder will find all files in the source directory, and encode them\n"
                     " into two files called (targetclassname).cpp and (targetclassname).h, which it\n"
                     " will write into the target directory supplied.\n\n"
                     " Any files in sub-directories of the source directory will be put into the\n"
                     " resultant class, but #ifdef'ed out using the name of the sub-directory (hard to\n"
                     " explain, but obvious when you try it...)\n";

        return 0;
    }

    const File sourceDirectory (File::getCurrentWorkingDirectory()
                                     .getChildFile (String (argv[1]).unquoted()));

    if (! sourceDirectory.isDirectory())
    {
        std::cout << "Source directory doesn't exist: "
                  << sourceDirectory.getFullPathName()
                  << std::endl << std::endl;

        return 0;
    }

    const File destDirectory (File::getCurrentWorkingDirectory()
                                   .getChildFile (String (argv[2]).unquoted()));

    if (! destDirectory.isDirectory())
    {
        std::cout << "Destination directory doesn't exist: "
                  << destDirectory.getFullPathName() << std::endl << std::endl;

        return 0;
    }

    String className (argv[3]);
    className = className.trim();

    const File headerFile (destDirectory.getChildFile (className).withFileExtension (".h"));
    const File cppFile    (destDirectory.getChildFile (className).withFileExtension (".cpp"));

    std::cout << "Creating " << headerFile.getFullPathName()
              << " and " << cppFile.getFullPathName()
              << " from files in " << sourceDirectory.getFullPathName()
              << "..." << std::endl << std::endl;

    auto files = sourceDirectory.findChildFiles (File::findFiles, true,
                                                 (argc > 4) ? argv[4] : "*");
    
    files.sort();
    if (files.isEmpty())
    {
        std::cout << "Didn't find any source files in: "
                  << sourceDirectory.getFullPathName() << std::endl << std::endl;
        return 0;
    }

    headerFile.deleteFile();
    cppFile.deleteFile();

    ScopedPointer<OutputStream> header (headerFile.createOutputStream());

    if (header == nullptr)
    {
        std::cout << "Couldn't open "
                  << headerFile.getFullPathName() << " for writing" << std::endl << std::endl;
        return 0;
    }

    ScopedPointer<OutputStream> cpp (cppFile.createOutputStream());

    if (cpp == nullptr)
    {
        std::cout << "Couldn't open "
                  << cppFile.getFullPathName() << " for writing" << std::endl << std::endl;
        return 0;
    }

    *header << "/* (Auto-generated binary data file (for parsing wavetables into matrix)). */\r\n"
               "/*index method: Saw = 0 , Sin 100, Tri 200, Saw = 300(repeat)*/\r\n\r\n"
               "extern double prerenderWabetable[303][512];\r\n";
               //"#pragma once\r\n\r\n"
               //"namespace " << className << "\r\n"
               //"{\r\n";

    *cpp << "/* (Auto-generated binary data file (for parsing wavetables into matrix)). */\r\n"
            "/*index method: Saw = 0 , Sin 100, Tri 200, Saw = 300(repeat)*/\r\n\r\n"
            "#include \"prerender.h\"\r\n\r\n"
            "double prerenderWabetable[303][512] = {\r\n";
    
    

    int totalBytes = 0;

    for (int i = 0; i < files.size(); ++i)
    {
        const File file (files[i]);

        // (avoid source control files and hidden files..)
        if (! isHiddenFile (file, sourceDirectory))
        {
//            if (file.getParentDirectory() != sourceDirectory)
//            {
//                *header << "  #ifdef " << file.getParentDirectory().getFileName().toUpperCase() << "\r\n";
//                *cpp << "#ifdef " << file.getParentDirectory().getFileName().toUpperCase() << "\r\n";
//
//                totalBytes += addFile (file, className, *header, *cpp);
//
//                *header << "  #endif\r\n";
//                *cpp << "#endif\r\n";
//            }
//            else
//            {
                totalBytes += addFile (file, className, *header, *cpp);
//            }
        }
    }

    *cpp << "};\r\n";

    header = nullptr;
    cpp = nullptr;

    std::cout << std::endl << " Total size of binary data: " << totalBytes << " bytes" << std::endl;

    return 0;
}