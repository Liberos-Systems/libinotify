#include <sys/inotify.h>
/* Events sent by the kernel.  */
/* /usr/include/sys/inotify.h */
namespace inotify {
   enum class InotifyEventsKernel : unsigned int {
       UNMOUNT=IN_UNMOUNT,     // Backing fs was unmounted. 0x00002000
       OVERFLOW=IN_Q_OVERFLOW, // Event queued overflowed. 0x00004000
       IGNORED=IN_IGNORED      // File was ignored. 0x00008000
   };
}
