#include "river3D_main.h"

#include "pd_path.h"
#include "pd_string_view.h"

StringView getProjectRoot
(
    StringView rootName
){
    StringView dot = pdCstrSV(".");
    char buf[4096];

    StringView current = pdExpandPath(dot, buf);

    for(; current.size < 4096;)
    {
        if(pdSVSame(current, rootName))
        {
            // riverLog(std::format("set project root to "PRI_SV" with length %u",
            //                      ARG_SV(current), current.size),
            //          RV_LOG_LEVEL_TRACE);
            //
            return current;
        }
        current = pdParentPath(current, buf);
    }

    StringView result = pdCstrSV("RV_PATH_UNDETERMINED");

    return result;
}
