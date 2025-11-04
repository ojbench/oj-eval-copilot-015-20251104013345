#include <cstdio>
#include <cstring>
#include <algorithm>

const char DATA_FILE[] = "data.bin";
const int MAX_ENTRIES = 10000;
const int COMPACT_THRESHOLD = 5000; // Compact after 5000 operations

struct Entry {
    char index[65];
    int value;
    char deleted;
};

class FileDatabase {
private:
    int operation_count;
    
    void compact() {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) return;
        
        // Read all entries and build final state
        Entry final_entries[MAX_ENTRIES];
        int final_count = 0;
        
        Entry entry;
        while (fread(&entry, sizeof(Entry), 1, fp) == 1) {
            if (entry.deleted == 0) {
                // Check if already exists
                bool found = false;
                for (int i = 0; i < final_count; i++) {
                    if (strcmp(final_entries[i].index, entry.index) == 0 &&
                        final_entries[i].value == entry.value) {
                        found = true;
                        break;
                    }
                }
                if (!found && final_count < MAX_ENTRIES) {
                    final_entries[final_count++] = entry;
                }
            } else {
                // Remove if exists
                for (int i = 0; i < final_count; i++) {
                    if (strcmp(final_entries[i].index, entry.index) == 0 &&
                        final_entries[i].value == entry.value) {
                        // Shift left
                        for (int j = i; j < final_count - 1; j++) {
                            final_entries[j] = final_entries[j + 1];
                        }
                        final_count--;
                        break;
                    }
                }
            }
        }
        fclose(fp);
        
        // Write compacted data
        fp = fopen(DATA_FILE, "wb");
        fwrite(final_entries, sizeof(Entry), final_count, fp);
        fclose(fp);
    }
    
public:
    FileDatabase() : operation_count(0) {
    }
    
    ~FileDatabase() {
    }
    
    void insert(const char* idx, int val) {
        // Append insert record
        FILE* fp = fopen(DATA_FILE, "ab");
        if (!fp) fp = fopen(DATA_FILE, "wb");
        
        Entry entry;
        strcpy(entry.index, idx);
        entry.value = val;
        entry.deleted = 0;
        fwrite(&entry, sizeof(Entry), 1, fp);
        fclose(fp);
        
        operation_count++;
        if (operation_count >= COMPACT_THRESHOLD) {
            compact();
            operation_count = 0;
        }
    }
    
    void remove(const char* idx, int val) {
        // Append delete record
        FILE* fp = fopen(DATA_FILE, "ab");
        if (!fp) return;
        
        Entry entry;
        strcpy(entry.index, idx);
        entry.value = val;
        entry.deleted = 1;
        fwrite(&entry, sizeof(Entry), 1, fp);
        fclose(fp);
        
        operation_count++;
        if (operation_count >= COMPACT_THRESHOLD) {
            compact();
            operation_count = 0;
        }
    }
    
    void find(const char* idx) {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) {
            printf("null\n");
            return;
        }
        
        // Build state by replaying log
        bool exists[MAX_ENTRIES];
        int values[MAX_ENTRIES];
        int count = 0;
        
        Entry entry;
        while (fread(&entry, sizeof(Entry), 1, fp) == 1) {
            if (strcmp(entry.index, idx) == 0) {
                if (entry.deleted == 0) {
                    // Check if already in list
                    int pos = -1;
                    for (int i = 0; i < count; i++) {
                        if (values[i] == entry.value) {
                            pos = i;
                            break;
                        }
                    }
                    if (pos == -1 && count < MAX_ENTRIES) {
                        // Add new
                        values[count] = entry.value;
                        exists[count] = true;
                        count++;
                    } else if (pos >= 0 && !exists[pos]) {
                        // Re-add deleted entry
                        exists[pos] = true;
                    }
                } else {
                    // Mark as deleted
                    for (int i = 0; i < count; i++) {
                        if (values[i] == entry.value) {
                            exists[i] = false;
                            break;
                        }
                    }
                }
            }
        }
        fclose(fp);
        
        // Collect results
        int results[MAX_ENTRIES];
        int res_count = 0;
        for (int i = 0; i < count; i++) {
            if (exists[i]) {
                results[res_count++] = values[i];
            }
        }
        
        if (res_count == 0) {
            printf("null\n");
        } else {
            std::sort(results, results + res_count);
            for (int i = 0; i < res_count; i++) {
                if (i > 0) printf(" ");
                printf("%d", results[i]);
            }
            printf("\n");
        }
    }
};

int main() {
    int n;
    scanf("%d", &n);
    
    FileDatabase db;
    
    for (int i = 0; i < n; i++) {
        char cmd[10];
        scanf("%s", cmd);
        
        if (strcmp(cmd, "insert") == 0) {
            char index[65];
            int value;
            scanf("%s%d", index, &value);
            db.insert(index, value);
        } else if (strcmp(cmd, "delete") == 0) {
            char index[65];
            int value;
            scanf("%s%d", index, &value);
            db.remove(index, value);
        } else if (strcmp(cmd, "find") == 0) {
            char index[65];
            scanf("%s", index);
            db.find(index);
        }
    }
    
    return 0;
}
