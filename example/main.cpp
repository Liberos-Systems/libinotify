#include <libinotify/libinotify.hpp>
#include <thread>
#include <csignal>

int main(int argc, char** argv)
{
  // Create a Watcher object
  inotify::Watcher watcher;

  // Set the directory to watch
  std::filesystem::path path("/home/kacper/Dokumenty/obs");
  watcher.recursive(path.string());

  // Run this in a separate thread, the main thread closes when 'q' is pressed
  std::atomic<bool> runWatcherThread(true);

  std::thread watcherThread([&]() {
    while (runWatcherThread) {
      auto events = watcher.getCurrentEvents();
      for (const auto& event : events) {
        std::cout << "Event: " << event << std::endl;
      }
    }
  });

  // Handle 'q' input to terminate the program
  char c;
  while (std::cin >> c) {
    if (c == 'q') {
      std::cout << "\nProgram terminated by user\n";
      runWatcherThread = false;
      if (watcherThread.joinable()) {
        watcherThread.join();
      }
      break;
    }
  }


  return 0;
}
