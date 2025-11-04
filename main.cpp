#include <cstdio>
#include <cstring>
#include <algorithm>

const char DATA_FILE[] = "data.bin";
const int MAX_ENTRIES = 1000;

struct Entry {
    char index[65];
    int value;
    char deleted;
};

class FileDatabase {
public:
    FileDatabase() {
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
                    if (pos == -1) {
                        // Add new
                        values[count] = entry.value;
                        exists[count] = true;
                        count++;
                    } else if (!exists[pos]) {
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
