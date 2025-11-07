// ReduceMZVList.cpp
// Compile: g++ -std=c++17 -fPIC -shared ReduceMZVList.cpp -o ReduceMZVList.dylib \
//    -I"${WOLFRAM_KERNEL}/SystemFiles/IncludeFiles/C" $(pkg-config --cflags --libs ginac)

#include "WolframLibrary.h"
#include <ginac/ginac.h>
#include <sstream>
#include <vector>
#include <string>

using namespace GiNaC;
using std::string;
using std::ostringstream;
using std::vector;

EXTERN_C DLLEXPORT int WolframLibrary_initialize(WolframLibraryData libData) {
    return LIBRARY_NO_ERROR;
}

// The main LibraryLink entry
EXTERN_C DLLEXPORT int ReduceMZVList(WolframLibraryData libData,
                                     mint Argc, MArgument *Args, MArgument Res) {
    if (Argc != 1) return LIBRARY_FUNCTION_ERROR;

    // Use thread-local storage to ensure string persists until Mathematica copies it
    static thread_local std::string resultStr;
    
    try {
        // Argument: a 2D tensor representing a list of lists of integers
        MTensor input = MArgument_getMTensor(Args[0]);
        if (!input) return LIBRARY_FUNCTION_ERROR;
        
        mint rank = libData->MTensor_getRank(input);
        
        parser reader;
        std::ostringstream output;
        
        if (rank == 2) {
            // 2D tensor: list of lists with fixed length
            const mint* dims = libData->MTensor_getDimensions(input);
            mint nLists = dims[0];
            mint sublistLen = dims[1];
            
            if (nLists < 0 || sublistLen < 0) {
                resultStr = "ERROR: Invalid tensor dimensions";
                MArgument_setUTF8String(Res, const_cast<char*>(resultStr.c_str()));
                return LIBRARY_FUNCTION_ERROR;
            }

            const mint* data = libData->MTensor_getIntegerData(input);
            if (!data) {
                resultStr = "ERROR: NULL tensor data";
                MArgument_setUTF8String(Res, const_cast<char*>(resultStr.c_str()));
                return LIBRARY_FUNCTION_ERROR;
            }

            mint offset = 0;
            for (mint i = 0; i < nLists; ++i) {
                // build comma-separated index string
                std::ostringstream indices;
                for (mint j = 0; j < sublistLen; ++j) {
                    if (j > 0) indices << ",";
                    indices << data[offset++];
                }
                
                string gcall = "G({" + indices.str() + "},1)";
                string reduce_call = "mzv_to_basis(" + gcall + ")";
                
                try {
                    ex reduced = reader(reduce_call);
                    output << gcall << " -> " << reduced;
                    if (i < nLists - 1) output << "\n";
                } catch (std::exception &e) {
                    output << gcall << " -> ERROR: " << e.what();
                    if (i < nLists - 1) output << "\n";
                }
            }
        } else {
            // Unsupported tensor rank
            resultStr = "ERROR: Expected a 2-dimensional tensor (list of lists)";
            MArgument_setUTF8String(Res, const_cast<char*>(resultStr.c_str()));
            return LIBRARY_FUNCTION_ERROR;
        }

        // Store result in thread-local string and return
        resultStr = output.str();
        MArgument_setUTF8String(Res, const_cast<char*>(resultStr.c_str()));
        return LIBRARY_NO_ERROR;
        
    } catch (std::exception &e) {
        resultStr = "ERROR: " + std::string(e.what());
        MArgument_setUTF8String(Res, const_cast<char*>(resultStr.c_str()));
        return LIBRARY_FUNCTION_ERROR;
    }
}