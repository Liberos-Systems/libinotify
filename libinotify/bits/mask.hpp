#include <sys/inotify.h>
/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH. */
/* /usr/include/sys/inotify.h */
namespace inotify {
    enum class InotifyMask : unsigned int {
        ACCESS=IN_ACCESS,               // File was accessed 0x00000001
        MODIFY=IN_MODIFY,               // File was modified 0x00000002
        ATTRIB=IN_ATTRIB,               // Metadata changed 0x00000004
        CLOSE_WRITE=IN_CLOSE_WRITE,     // Writable file was closed 0x00000008
        CLOSE_NOWRITE=IN_CLOSE_NOWRITE, // Unwritable file was closed 0x00000010
        CLOSE=IN_CLOSE,                 // File was closed (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
        OPEN=IN_OPEN,                   // File was opened 0x00000020
        MOVED_FROM=IN_MOVED_FROM,       // File was moved from X 0x00000040
        MOVED_TO=IN_MOVED_TO,           // File was moved to Y 0x00000080
        MOVE=IN_MOVE,                   // File was moved (IN_MOVED_FROM | IN_MOVED_TO)
        CREATE=IN_CREATE,               // Subfile was created 0x00000100
        DELETE=IN_DELETE,               // Subfile was deleted 0x00000200
        DELETE_SELF=IN_DELETE_SELF,     // Self was deleted 0x00000400
        MOVE_SELF=IN_MOVE_SELF          // Self was moved 0x00000800
    };
}
