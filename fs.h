#define DISACTIVE 0
#define ACTIVE 1
#define COMMON_COUNT_BLOCKS 1024
#define SIZE_BLOCK 128
#define COUNT_ENTITY 128
#define COUNT_BLOCKS_IN_ENTITY 8

typedef struct entity {
  char title[128];
  short operations; // 0x1 -- read, 0x2 - w, 0x4 - d; 0x7 - rwd
  unsigned int time_creation;
  unsigned int size;
  short is_file;
  char full_name[4096];
  unsigned int count_blocks;
  // 8 * 128 = 1Kb
  int indexes[COUNT_BLOCKS_IN_ENTITY]; // directory - пусть в data блока хранятся первые индексы других блоков entity
  // block blocks[32];
  unsigned int count_entities;
  unsigned int self_index; // id файла, индекс в sb
} entity;

typedef struct super_block {
  unsigned int count_blocks;
  unsigned int free_blocks[COMMON_COUNT_BLOCKS];
  unsigned int free_entity[COUNT_ENTITY]; // [1] - свободен
  short size_block;
  unsigned int id_file_system;
  short state;
  entity entities[COUNT_ENTITY];
} super_block;

super_block sb;

typedef struct block {
  char data[SIZE_BLOCK];
  unsigned int end_data; // номер символа в блоке
} block;