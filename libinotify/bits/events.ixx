export module events;
#include <sys/inotify.h>

/* Helper events.  */
/* /usr/include/sys/inotify.h */
export namespace inotify {
   enum class InotifyHelperEvents : unsigned int {
       CLOSE = IN_CLOSE,          // Close. (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
       MOVE = IN_MOVE,            // Moves. (IN_MOVED_FROM | IN_MOVED_TO)
       ALL_EVENTS = IN_ALL_EVENTS // All events which a program can wait on(IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE  
                                  // IN_CLOSE_NOWRITE | IN_OPEN | IN_MOVED_FROM 
                                  // IN_MOVED_TO | IN_CREATE | IN_DELETE 
                                  // IN_DELETE_SELF | IN_MOVE_SELF)
   };
}
