#include "freelist.h"

#include "core/systems/fmemory.h"
#include "core/systems/logger.h"

/*
 * This is a mish-mash of about 50 billion stackoverflow/github repos. Good luck
 * changing this.
 */

typedef struct freelistNode {
    u64 offset;
    u64 size;
    struct freelistNode* next;
} freelistNode;

typedef struct internalState {
    u64 totalSize;
    u64 maxEntries;
    freelistNode* head;
    freelistNode* nodes;
} internalState;

freelistNode* getNode(freelist* list);
void invalidateNode(freelist* list, freelistNode* node);

void freelistCreate(u64 totalSize, u64* memoryRequirement, void* memory,
                    freelist* outList) {
    // Find the max entries this freelist can have to est. the memReq
    u64 maxEntries = (totalSize / sizeof(void*));

    *memoryRequirement =
        sizeof(internalState) + (sizeof(freelistNode) * maxEntries);

    if (!memory) {
        return;
    }

    outList->memory = memory;
    outList->memorySize = *memoryRequirement;

    fzeroMemory(outList->memory, *memoryRequirement);

    internalState* state = outList->memory;
    state->nodes = (void*)(outList->memory + sizeof(internalState));
    state->maxEntries = maxEntries;
    state->totalSize = totalSize;

    state->head = &state->nodes[0];
    state->head->offset = 0;
    state->head->size = totalSize;
    state->head->next = 0;

    // Invalidate the nodes. This is how we will find an available one when we
    // need it.
    for (u64 i = 1; i < state->maxEntries; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }
}

b8 freelistAllocateBlock(freelist* list, u64 size, u64* outOffset) {
    if (!list || !outOffset || !list->memory) {
        return false;
    }
    internalState* state = list->memory;
    freelistNode* node = state->head;
    freelistNode* previous = 0;
    while (node) {
        if (node->size == size) {
            // Exact match. Just return the node.
            *outOffset = node->offset;
            freelistNode* nodeToReturn = 0;
            if (previous) {
                previous->next = node->next;
                nodeToReturn = node;
            } else {
                // This node is the head of the list. Reassign the head
                // and return the previous head node.
                nodeToReturn = state->head;
                state->head = node->next;
            }
            invalidateNode(list, nodeToReturn);
            return true;
        } else if (node->size > size) {
            // Node is larger. Deduct the memory from it and move the offset
            // by that amount.
            *outOffset = node->offset;
            node->size -= size;
            node->offset += size;
            return true;
        }

        previous = node;
        node = node->next;
    }

    u64 freeSpace = freelistFreeSpace(list);
    FWARN("freelistFindBlock, no block large enough found (requested: %lluB, "
          "available: %lluB).",
          size, freeSpace);
    return false;
}

b8 freelistFreeBlock(freelist* list, u64 size, u64 offset) {
    if (!list || !list->memory || !size) {
        return false;
    }
    internalState* state = list->memory;
    freelistNode* n = state->head;
    freelistNode* previous = 0;
    if (!n) {
        // If the whole thing is filled then we need another node at the start
        freelistNode* nn = getNode(list);
        nn->next = 0;
        nn->size = size;
        nn->offset = offset;
        state->head = nn;
        return true;
    } else {
        while (n) {
            if (n->offset == offset) {
                n->size += size;

                // If two nodes get connected then we can connect them and
                // delete one
                if (n->next && n->next->offset == n->offset + n->size) {
                    n->size += n->next->size;
                    freelistNode* next = n->next;
                    n->next = n->next->next;
                    invalidateNode(list, next);
                }
                return true;
            } else if (n->offset > offset) {
                freelistNode* newNode = getNode(list);
                newNode->offset = offset;
                newNode->size = size;

                // If their is a previous node update it's next var
                if (previous) {
                    previous->next = newNode;
                    newNode->next = n;
                } else {
                    newNode->next = n;
                    state->head = newNode;
                }

                // See if `newNode` can be connected to `n`
                if (newNode->next &&
                    newNode->offset + newNode->size == newNode->next->offset) {
                    newNode->size += newNode->next->size;
                    freelistNode* deletedNode = newNode->next;
                    newNode->next = deletedNode->next;
                    invalidateNode(list, deletedNode);
                }

                // See if `newNode` can be connected to `previous`
                if (previous &&
                    previous->offset + previous->size == newNode->offset) {
                    previous->size += newNode->size;
                    freelistNode* deletedNode = newNode;
                    previous->next = deletedNode->next;
                    invalidateNode(list, deletedNode);
                }

                return true;
            }

            previous = n;
            n = n->next;
        }
    }

    FWARN("Unable to find block to be freed. Function could be written wrong "
          "or the dev "
          "who called this function is stupid, who knows what is true.");
    return false;
}

b8 freelistResize(freelist* list, u64* memoryReq, u64 size, void* newMemory,
                  void* outOldMemory) {
    // Find the max entries this freelist can have to est. the memReq
    u64 maxEntries = (size / sizeof(void*));

    *memoryReq = sizeof(internalState) + (sizeof(freelistNode) * maxEntries);

    if (!newMemory) {
        return true;
    }

    outOldMemory = list->memory;
    internalState* oldState = (internalState*)list->memory;
    u64 sizeDiff = size - oldState->totalSize;
    list->memory = newMemory;

    // The block's layout is head* first, then array of available nodes.
    fzeroMemory(list->memory, *memoryReq);

    // Setup the new state.
    internalState* state = (internalState*)list->memory;
    state->nodes = (void*)(list->memory + sizeof(internalState));
    state->maxEntries = maxEntries;
    state->totalSize = size;

    // Invalidate the nodes. This is how we will find an available one when we
    // need it.
    for (u64 i = 1; i < state->maxEntries; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }

    state->head = &state->nodes[0];

    // Copy over the nodes.
    freelistNode* newListNode = state->head;
    freelistNode* nodeIter = oldState->head;
    if (!nodeIter) {
        state->head->offset = oldState->totalSize;
        state->head->size = sizeDiff;
        state->head->next = 0;
    } else {
        while (nodeIter) {
            freelistNode* newNode = getNode(list);
            newNode->next = 0;
            newNode->size = nodeIter->size;
            newNode->offset = nodeIter->offset;
            newListNode->next = newNode;
            // Move to the next entry.
            newListNode = newListNode->next;

            if (nodeIter->next) {
                nodeIter = nodeIter->next;
            } else {
                if (nodeIter->offset + nodeIter->size == oldState->totalSize) {
                    newNode->size += sizeDiff;
                } else {
                    freelistNode* newNodeEnd = getNode(list);
                    newNodeEnd->next = 0;
                    newNodeEnd->size = sizeDiff;
                    newNodeEnd->offset = oldState->totalSize;
                    newNode->next = newNodeEnd;
                }
                break;
            }
        }
    }

    return true;
}

void freelistClear(freelist* list) {
    if (!list || !list->memory) {
        return;
    }

    internalState* state = list->memory;
    // Invalidate the nodes. This is how we will find an available one when we
    // need it.
    for (u64 i = 1; i < state->maxEntries; ++i) {
        state->nodes[i].offset = INVALID_ID;
        state->nodes[i].size = INVALID_ID;
    }

    // Reset the head to occupy the entire thing.
    state->head->offset = 0;
    state->head->size = state->totalSize;
    state->head->next = 0;
}

u64 freelistFreeSpace(freelist* list) {
    if (!list || !list->memory) {
        return 0;
    }

    u64 total = 0;
    internalState* state = list->memory;
    freelistNode* node = state->head;
    while (node) {
        total += node->size;
        node = node->next;
    }

    return total;
}

freelistNode* getNode(freelist* list) {
    internalState* state = list->memory;
    for (u64 i = 1; i < state->maxEntries; ++i) {
        if (state->nodes[i].offset == INVALID_ID) {
            return &state->nodes[i];
        }
    }

    // Return nothing if no nodes are available.
    return 0;
}

void invalidateNode(freelist* list, freelistNode* node) {
    node->offset = INVALID_ID;
    node->size = INVALID_ID;
    node->next = 0;
}
