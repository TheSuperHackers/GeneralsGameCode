export module engine.navigation.reference.request_queue;

export namespace navigation::reference {

inline constexpr unsigned int CellsPerFrame = 5000;
inline constexpr int RequestQueueLength = 512;

// Operate on the existing array and indices so snapshots retain their layout.
// One slot remains unused to distinguish an empty queue from a full queue.
template<class Id, int Capacity>
bool queueRequest(Id (&requests)[Capacity], int head, int& tail, Id id)
{
    int slot = head;
    while (slot != tail) {
        if (requests[slot] == id) return true;
        slot++;
        if (slot >= Capacity) slot = 0;
    }

    int nextSlot = tail + 1;
    if (nextSlot >= Capacity) nextSlot = 0;
    if (nextSlot == head) return false;
    requests[tail] = id;
    tail = nextSlot;
    return true;
}

// This is a budget between requests, not a cap within a single search.
// Resolve before clearing the slot; clear before invoking the request; advance
// the head afterwards. Callbacks may enqueue more work or allocate more cells.
template<class Id, int Capacity, class Resolve, class Process>
int processRequests(Id (&requests)[Capacity], int& head, int& tail,
                    int& cellsAllocated, Id invalidId,
                    Resolve resolve, Process process)
{
    int pathsFound = 0;
    while (cellsAllocated < CellsPerFrame && tail != head) {
        auto* object = resolve(requests[head]);
        requests[head] = invalidId;
        if (object) {
            if (process(object)) pathsFound++;
        }
        head = head + 1;
        if (head >= Capacity) head = 0;
    }
    return pathsFound;
}

} // namespace navigation
