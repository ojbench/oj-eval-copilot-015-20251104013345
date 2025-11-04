#include <cstdio>
#include <cstring>
#include <algorithm>

const int BLOCK_SIZE = 500;
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

struct Block {
    int size;
    Entry entries[BLOCK_SIZE];
    
    void sort() {
        std::sort(entries, entries + size);
    }
};

class FileDatabase {
private:
    
    int read_num_blocks() {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) return 0;
        int n;
        fread(&n, sizeof(int), 1, fp);
        fclose(fp);
        return n;
    }
    
    void read_block(int idx, Block& block) {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) {
            block.size = 0;
            return;
        }
        
        int n;
        fread(&n, sizeof(int), 1, fp);
        
        // Skip to the idx-th block
        for (int i = 0; i < idx; i++) {
            int sz;
            fread(&sz, sizeof(int), 1, fp);
            fseek(fp, sizeof(Entry) * sz, SEEK_CUR);
        }
        
        fread(&block.size, sizeof(int), 1, fp);
        fread(block.entries, sizeof(Entry), block.size, fp);
        fclose(fp);
    }
    
    void write_all_blocks(Block* blocks, int num_blocks) {
        FILE* fp = fopen(DATA_FILE, "wb");
        fwrite(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            fwrite(&blocks[i].size, sizeof(int), 1, fp);
            fwrite(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
    }
    
    void rebuild() {
        int num_blocks = read_num_blocks();
        if (num_blocks == 0) return;
        
        // Read all blocks
        Block* blocks = new Block[num_blocks + 10];
        FILE* fp = fopen(DATA_FILE, "rb");
        fread(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            fread(&blocks[i].size, sizeof(int), 1, fp);
            fread(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
        
        // Rebuild: merge and split
        Block* new_blocks = new Block[num_blocks * 2];
        int new_count = 0;
        Block current;
        current.size = 0;
        
        for (int i = 0; i < num_blocks; i++) {
            for (int j = 0; j < blocks[i].size; j++) {
                current.entries[current.size++] = blocks[i].entries[j];
                
                if (current.size >= BLOCK_SIZE) {
                    current.sort();
                    new_blocks[new_count++] = current;
                    current.size = 0;
                }
            }
        }
        
        if (current.size > 0) {
            current.sort();
            new_blocks[new_count++] = current;
        }
        
        write_all_blocks(new_blocks, new_count);
        
        delete[] blocks;
        delete[] new_blocks;
    }
    
public:
    FileDatabase() {
    }
    
    ~FileDatabase() {
    }
    
    void insert(const char* idx, int val) {
        int num_blocks = read_num_blocks();
        
        if (num_blocks == 0) {
            // Create first block
            Block block;
            block.size = 1;
            strcpy(block.entries[0].index, idx);
            block.entries[0].value = val;
            write_all_blocks(&block, 1);
            return;
        }
        
        // Read all blocks (needed to check duplicates and find insertion point)
        Block* blocks = new Block[num_blocks + 1];
        FILE* fp = fopen(DATA_FILE, "rb");
        fread(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            fread(&blocks[i].size, sizeof(int), 1, fp);
            fread(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
        
        // Check for duplicates
        for (int i = 0; i < num_blocks; i++) {
            for (int j = 0; j < blocks[i].size; j++) {
                if (strcmp(blocks[i].entries[j].index, idx) == 0 && 
                    blocks[i].entries[j].value == val) {
                    delete[] blocks;
                    return; // Already exists
                }
            }
        }
        
        // Find block to insert into
        int target_block = -1;
        for (int i = 0; i < num_blocks; i++) {
            if (blocks[i].size < BLOCK_SIZE) {
                target_block = i;
                break;
            }
        }
        
        if (target_block == -1) {
            // All blocks full, create new one
            target_block = num_blocks;
            blocks[num_blocks].size = 0;
            num_blocks++;
        }
        
        // Insert entry
        strcpy(blocks[target_block].entries[blocks[target_block].size].index, idx);
        blocks[target_block].entries[blocks[target_block].size].value = val;
        blocks[target_block].size++;
        blocks[target_block].sort();
        
        write_all_blocks(blocks, num_blocks);
        delete[] blocks;
        
        // Rebuild if too many blocks
        if (num_blocks > 100) {
            rebuild();
        }
    }
    
    void remove(const char* idx, int val) {
        int num_blocks = read_num_blocks();
        if (num_blocks == 0) return;
        
        Block* blocks = new Block[num_blocks];
        FILE* fp = fopen(DATA_FILE, "rb");
        fread(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            fread(&blocks[i].size, sizeof(int), 1, fp);
            fread(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
        
        // Find and remove
        for (int i = 0; i < num_blocks; i++) {
            for (int j = 0; j < blocks[i].size; j++) {
                if (strcmp(blocks[i].entries[j].index, idx) == 0 && 
                    blocks[i].entries[j].value == val) {
                    // Remove by shifting
                    for (int k = j; k < blocks[i].size - 1; k++) {
                        blocks[i].entries[k] = blocks[i].entries[k + 1];
                    }
                    blocks[i].size--;
                    write_all_blocks(blocks, num_blocks);
                    delete[] blocks;
                    return;
                }
            }
        }
        
        delete[] blocks;
    }
    
    void find(const char* idx) {
        int num_blocks = read_num_blocks();
        if (num_blocks == 0) {
            printf("null\n");
            return;
        }
        
        int results[10000];
        int count = 0;
        
        FILE* fp = fopen(DATA_FILE, "rb");
        fread(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            Block block;
            fread(&block.size, sizeof(int), 1, fp);
            fread(block.entries, sizeof(Entry), block.size, fp);
            
            for (int j = 0; j < block.size; j++) {
                if (strcmp(block.entries[j].index, idx) == 0) {
                    results[count++] = block.entries[j].value;
                }
            }
        }
        fclose(fp);
        
        if (count == 0) {
            printf("null\n");
        } else {
            std::sort(results, results + count);
            for (int i = 0; i < count; i++) {
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
