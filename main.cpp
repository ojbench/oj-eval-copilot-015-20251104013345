#include <cstdio>
#include <cstring>
#include <algorithm>

const int BLOCK_SIZE = 500;
const int MAX_BLOCKS = 250;
const char DATA_FILE[] = "data.bin";

struct Entry {
    char index[65];
    int value;
    
    bool operator<(const Entry& other) const {
        int cmp = strcmp(index, other.index);
        if (cmp != 0) return cmp < 0;
        return value < other.value;
    }
    
    bool operator==(const Entry& other) const {
        return strcmp(index, other.index) == 0 && value == other.value;
    }
};

struct BlockHeader {
    int size;
};

class FileDatabase {
private:
    
    int get_num_blocks() {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) return 0;
        int n;
        fread(&n, sizeof(int), 1, fp);
        fclose(fp);
        return n;
    }
    
    void read_all_data(Entry* all_entries, int& total_count) {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) {
            total_count = 0;
            return;
        }
        
        int num_blocks;
        fread(&num_blocks, sizeof(int), 1, fp);
        
        total_count = 0;
        for (int i = 0; i < num_blocks; i++) {
            BlockHeader header;
            fread(&header, sizeof(BlockHeader), 1, fp);
            fread(&all_entries[total_count], sizeof(Entry), header.size, fp);
            total_count += header.size;
        }
        fclose(fp);
    }
    
    void write_all_data(Entry* all_entries, int total_count) {
        // Organize into blocks
        int num_blocks = (total_count + BLOCK_SIZE - 1) / BLOCK_SIZE;
        
        FILE* fp = fopen(DATA_FILE, "wb");
        fwrite(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            int start = i * BLOCK_SIZE;
            int block_size = (i == num_blocks - 1) ? (total_count - start) : BLOCK_SIZE;
            
            BlockHeader header;
            header.size = block_size;
            fwrite(&header, sizeof(BlockHeader), 1, fp);
            fwrite(&all_entries[start], sizeof(Entry), block_size, fp);
        }
        fclose(fp);
    }
    
public:
    FileDatabase() {
    }
    
    ~FileDatabase() {
    }
    
    void insert(const char* idx, int val) {
        Entry* all_entries = new Entry[MAX_BLOCKS * BLOCK_SIZE];
        int total_count;
        read_all_data(all_entries, total_count);
        
        // Check for duplicates
        for (int i = 0; i < total_count; i++) {
            if (strcmp(all_entries[i].index, idx) == 0 && all_entries[i].value == val) {
                delete[] all_entries;
                return; // Already exists
            }
        }
        
        // Add new entry
        strcpy(all_entries[total_count].index, idx);
        all_entries[total_count].value = val;
        total_count++;
        
        write_all_data(all_entries, total_count);
        delete[] all_entries;
    }
    
    void remove(const char* idx, int val) {
        Entry* all_entries = new Entry[MAX_BLOCKS * BLOCK_SIZE];
        int total_count;
        read_all_data(all_entries, total_count);
        
        // Find and remove
        for (int i = 0; i < total_count; i++) {
            if (strcmp(all_entries[i].index, idx) == 0 && all_entries[i].value == val) {
                // Shift left
                for (int j = i; j < total_count - 1; j++) {
                    all_entries[j] = all_entries[j + 1];
                }
                total_count--;
                write_all_data(all_entries, total_count);
                delete[] all_entries;
                return;
            }
        }
        delete[] all_entries;
    }
    
    void find(const char* idx) {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) {
            printf("null\n");
            return;
        }
        
        int num_blocks;
        fread(&num_blocks, sizeof(int), 1, fp);
        
        int results[10000];
        int res_count = 0;
        
        for (int i = 0; i < num_blocks; i++) {
            BlockHeader header;
            fread(&header, sizeof(BlockHeader), 1, fp);
            
            Entry entries[BLOCK_SIZE];
            fread(entries, sizeof(Entry), header.size, fp);
            
            for (int j = 0; j < header.size; j++) {
                if (strcmp(entries[j].index, idx) == 0) {
                    results[res_count++] = entries[j].value;
                }
            }
        }
        fclose(fp);
        
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
