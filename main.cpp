#include <cstdio>
#include <cstring>
#include <algorithm>

const char DATA_FILE[] = "data.bin";
const int MAX_BUFFER = 1000;

struct Entry {
    char index[65];
    int value;
    char deleted; // 0 = active, 1 = deleted
    
    bool operator<(const Entry& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
};

class FileDatabase {
private:
    int operation_count;
    
public:
    FileDatabase() : operation_count(0) {
    }
    
    ~FileDatabase() {
    }
    
    void insert(const char* idx, int val) {
        // Check if already exists
        FILE* fp = fopen(DATA_FILE, "rb");
        if (fp) {
            Entry entry;
            while (fread(&entry, sizeof(Entry), 1, fp) == 1) {
                if (strcmp(entry.index, idx) == 0 && entry.value == val && entry.deleted == 0) {
                    fclose(fp);
                    return; // Already exists
                }
            }
            fclose(fp);
        }
        
        // Append new entry
        fp = fopen(DATA_FILE, "ab");
        Entry entry;
        strcpy(entry.index, idx);
        entry.value = val;
        entry.deleted = 0;
        fwrite(&entry, sizeof(Entry), 1, fp);
        fclose(fp);
        
        operation_count++;
        if (operation_count >= 500) {
            compact();
            operation_count = 0;
        }
    }
    
    void remove(const char* idx, int val) {
        // Mark as deleted by appending a deletion marker
        FILE* fp = fopen(DATA_FILE, "ab");
        if (!fp) return;
        
        Entry entry;
        strcpy(entry.index, idx);
        entry.value = val;
        entry.deleted = 1;
        fwrite(&entry, sizeof(Entry), 1, fp);
        fclose(fp);
        
        operation_count++;
        if (operation_count >= 500) {
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
        
        // Use an array to track active entries
        int values[MAX_BUFFER];
        int count = 0;
        
        Entry entry;
        while (fread(&entry, sizeof(Entry), 1, fp) == 1) {
            if (strcmp(entry.index, idx) == 0) {
                if (entry.deleted == 0) {
                    // Add to results if not already there
                    bool found = false;
                    for (int i = 0; i < count; i++) {
                        if (values[i] == entry.value) {
                            found = true;
                            break;
                        }
                    }
                    if (!found && count < MAX_BUFFER) {
                        values[count++] = entry.value;
                    }
                } else {
                    // Remove from results
                    for (int i = 0; i < count; i++) {
                        if (values[i] == entry.value) {
                            for (int j = i; j < count - 1; j++) {
                                values[j] = values[j + 1];
                            }
                            count--;
                            break;
                        }
                    }
                }
            }
        }
        fclose(fp);
        
        if (count == 0) {
            printf("null\n");
        } else {
            std::sort(values, values + count);
            for (int i = 0; i < count; i++) {
                if (i > 0) printf(" ");
                printf("%d", values[i]);
            }
            printf("\n");
        }
    }
    
    void compact() {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) return;
        
        // Read all entries
        Entry entries[MAX_BUFFER];
        int count = 0;
        Entry entry;
        
        while (fread(&entry, sizeof(Entry), 1, fp) == 1 && count < MAX_BUFFER) {
            if (entry.deleted == 0) {
                // Check if this entry is later deleted
                bool is_deleted = false;
                long pos = ftell(fp);
                Entry later;
                while (fread(&later, sizeof(Entry), 1, fp) == 1) {
                    if (strcmp(later.index, entry.index) == 0 && 
                        later.value == entry.value && later.deleted == 1) {
                        is_deleted = true;
                        break;
                    }
                }
                fseek(fp, pos, SEEK_SET);
                
                if (!is_deleted) {
                    // Check if already in array (duplicates)
                    bool found = false;
                    for (int i = 0; i < count; i++) {
                        if (strcmp(entries[i].index, entry.index) == 0 && 
                            entries[i].value == entry.value) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        entries[count++] = entry;
                    }
                }
            }
        }
        fclose(fp);
        
        // Write back compacted data
        fp = fopen(DATA_FILE, "wb");
        fwrite(entries, sizeof(Entry), count, fp);
        fclose(fp);
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
