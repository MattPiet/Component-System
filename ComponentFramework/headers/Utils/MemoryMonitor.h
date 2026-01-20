#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>


struct MemoryTag {
    const char* file;
    int line;
    std::size_t size;
    bool isArray;
};


inline std::vector<std::pair<void*, MemoryTag>> memoryTracker;
inline std::size_t currentUsage = 0;


inline void PrintMemoryAction(void* ptr, std::size_t size, const MemoryTag& tag, const char* action) {
    std::cout << "========================================\n"
        << "[MEMORY " << action << "]\n"
        << "File: " << tag.file << " (Line: " << tag.line << ")\n"
        << "Addr: " << ptr << " | Size: " << size << " bytes " << (tag.isArray ? "[ARRAY]" : "[OBJECT]") << "\n"
        << "Total Before: " << currentUsage << " bytes\n";

    if (std::string(action) == "DELETED") {
        currentUsage -= size;
    }
    else {
        currentUsage += size;
    }

    std::cout << "Total After:  " << currentUsage << " bytes\n"
        << "========================================\n\n";
}


inline void* operator new(std::size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (ptr) {
        MemoryTag tag = { file, line, size, false };
        memoryTracker.push_back({ ptr, tag });
        PrintMemoryAction(ptr, size, tag, "ALLOCATED");
    }
    return ptr;
}

inline void* operator new[](std::size_t size, const char* file, int line) {
    void* ptr = std::malloc(size);
    if (ptr) {
        MemoryTag tag = { file, line, size, true };
        memoryTracker.push_back({ ptr, tag });
        PrintMemoryAction(ptr, size, tag, "ALLOCATED");
    }
    return ptr;
}

inline void operator delete(void* ptr, std::size_t size) noexcept {
    if (!ptr) return;
    auto it = std::find_if(memoryTracker.begin(), memoryTracker.end(),
        [ptr](const auto& pair) { return pair.first == ptr; });

    if (it != memoryTracker.end()) {
        PrintMemoryAction(ptr, size, it->second, "DELETED");
        memoryTracker.erase(it);
    }
    std::free(ptr);
}

inline void operator delete[](void* ptr) noexcept {
    if (!ptr) return;
    auto it = std::find_if(memoryTracker.begin(), memoryTracker.end(),
        [ptr](const auto& pair) { return pair.first == ptr; });

    if (it != memoryTracker.end()) {
        PrintMemoryAction(ptr, it->second.size, it->second, "DELETED");
        memoryTracker.erase(it);
    }
    std::free(ptr);
}

inline static void ReportLeaks() {
    std::cout << "\n--- FINAL MEMORY LEAK REPORT ---\n";
    if (memoryTracker.empty()) {
        std::cout << "Status: CLEAN - All memory deallocated.\n";
    }
    else {
        std::cout << "Status: LEAKS DETECTED (" << memoryTracker.size() << " items)\n";
        for (const auto& [ptr, tag] : memoryTracker) {
            std::cout << "-> " << (tag.isArray ? "Array " : "Object")
                << " leaked from " << tag.file << ":" << tag.line
                << " (" << tag.size << " bytes)\n";
        }
    }
    std::cout << "--------------------------------\n\n";
}

#define new new(__FILE__, __LINE__)

#endif