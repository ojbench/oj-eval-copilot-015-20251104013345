#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>

const int BLOCK_SIZE = 320;
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
    
    bool is_full() const {
        return size >= BLOCK_SIZE;
    }
};

class FileDatabase {
private:
    int num_blocks;
    
    void load_blocks(std::vector<Block>& blocks) {
        FILE* fp = fopen(DATA_FILE, "rb");
        if (!fp) {
            blocks.clear();
            Block empty;
            empty.size = 0;
            blocks.push_back(empty);
            return;
        }
        
        fread(&num_blocks, sizeof(int), 1, fp);
        blocks.resize(num_blocks);
        for (int i = 0; i < num_blocks; i++) {
            fread(&blocks[i].size, sizeof(int), 1, fp);
            fread(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
    }
    
    void save_blocks(const std::vector<Block>& blocks) {
        FILE* fp = fopen(DATA_FILE, "wb");
        num_blocks = blocks.size();
        fwrite(&num_blocks, sizeof(int), 1, fp);
        
        for (int i = 0; i < num_blocks; i++) {
            fwrite(&blocks[i].size, sizeof(int), 1, fp);
            fwrite(blocks[i].entries, sizeof(Entry), blocks[i].size, fp);
        }
        fclose(fp);
    }
    
    void rebuild_if_needed(std::vector<Block>& blocks) {
        // Merge small blocks and split large blocks
        std::vector<Block> new_blocks;
        Block current;
        current.size = 0;
        
        for (auto& block : blocks) {
            for (int i = 0; i < block.size; i++) {
                current.entries[current.size++] = block.entries[i];
                
                if (current.size >= BLOCK_SIZE) {
                    current.sort();
                    new_blocks.push_back(current);
                    current.size = 0;
                }
            }
        }
        
        if (current.size > 0) {
            current.sort();
            new_blocks.push_back(current);
        }
        
        if (new_blocks.empty()) {
            Block empty;
            empty.size = 0;
            new_blocks.push_back(empty);
        }
        
        blocks = new_blocks;
    }
    
public:
    FileDatabase() {
        num_blocks = 0;
    }
    
    ~FileDatabase() {
    }
    
    void insert(const char* index, int value) {
        std::vector<Block> blocks;
        load_blocks(blocks);
        
        Entry new_entry;
        strcpy(new_entry.index, index);
        new_entry.value = value;
        
        // Check if entry already exists
        for (auto& block : blocks) {
            for (int i = 0; i < block.size; i++) {
                if (block.entries[i] == new_entry) {
                    return; // Already exists
                }
            }
        }
        
        // Insert into appropriate block
        bool inserted = false;
        for (auto& block : blocks) {
            if (!block.is_full()) {
                block.entries[block.size++] = new_entry;
                block.sort();
                inserted = true;
                break;
            }
        }
        
        if (!inserted) {
            Block new_block;
            new_block.size = 1;
            new_block.entries[0] = new_entry;
            blocks.push_back(new_block);
        }
        
        if (blocks.size() > 50) {
            rebuild_if_needed(blocks);
        }
        
        save_blocks(blocks);
    }
    
    void remove(const char* index, int value) {
        std::vector<Block> blocks;
        load_blocks(blocks);
        
        Entry target;
        strcpy(target.index, index);
        target.value = value;
        
        for (auto& block : blocks) {
            for (int i = 0; i < block.size; i++) {
                if (block.entries[i] == target) {
                    // Remove by shifting
                    for (int j = i; j < block.size - 1; j++) {
                        block.entries[j] = block.entries[j + 1];
                    }
                    block.size--;
                    save_blocks(blocks);
                    return;
                }
            }
        }
    }
    
    void find(const char* index) {
        std::vector<Block> blocks;
        load_blocks(blocks);
        
        std::vector<int> results;
        
        for (auto& block : blocks) {
            for (int i = 0; i < block.size; i++) {
                if (strcmp(block.entries[i].index, index) == 0) {
                    results.push_back(block.entries[i].value);
                }
            }
        }
        
        if (results.empty()) {
            printf("null\n");
        } else {
            std::sort(results.begin(), results.end());
            for (size_t i = 0; i < results.size(); i++) {
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
