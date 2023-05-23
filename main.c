#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fs.h"

void load_super_block(FILE *fs) {
  fseek(fs, 0, SEEK_SET);
  char flag = fwrite(&sb, sizeof(sb), 1, fs);
  if (!flag) {
    printf("\nWrite sb error\n");
    exit(1);
  }
}

void run_file_system(FILE *fs) {  // считывает суперблок если фс не пустая, иначе создает и инициализирует его 
  fseek(fs, 0, SEEK_END);

  if (ftell(fs) > 0) {
    fseek(fs, 0, SEEK_SET);
    fread(&sb, sizeof(sb), 1, fs);
  } else {
    for (size_t i = 0; i < COUNT_ENTITY; i++) sb.free_entity[i] = 1;
    for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) sb.free_blocks[i] = 1;

    sb.count_blocks = COMMON_COUNT_BLOCKS;
    sb.id_file_system = 1;
    sb.size_block = SIZE_BLOCK;
  }
  sb.state = ACTIVE;
}

entity create_file(FILE *fs, char title[128], short ops) {
  entity file;

  file.count_blocks = 0;
  strcpy(file.full_name, "FullName");
  strcpy(file.title, title);
  file.operations = ops;
  file.size = 0;
  file.is_file = 1;
  file.time_creation = time(NULL);

  for (size_t i = 0; i < COUNT_BLOCKS_IN_ENTITY; i++) {
    file.indexes[i] = -1; // значит свободен
  }
  
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (sb.free_entity[i]) {
      printf("[%d] entity\n", i);
      file.self_index = i;
      sb.entities[i] = file;
      sb.free_entity[i] = 0;
      break;
    }
  }

  return file;
}

block create_block(FILE *fs, char data[128]) {
  block _block;
  _block.data[0] = '\0';
  int i = 0;
  for (; data[i] != '\0'; i++) { //+проверка на переполнение блока
    _block.data[i] = data[i];
  }
  _block.data[i] = '\0'; // тут тоже проверка

  return _block;
}

void write_block(FILE *fs, block _block) {
  for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) { // + проверка на переполнение??
    if (sb.free_blocks[i]) {
      fseek(fs, sizeof(super_block) + sizeof(_block) * i, SEEK_SET);
      char flag = fwrite(&_block, sizeof(_block), 1, fs);

      if (!flag) {
        printf("\nWrite block error\n");
        exit(1);
      }
      sb.free_blocks[i] = 0;
      break;
    }
  }
}

block get_block(FILE *fs, unsigned int index) {
  block _block;

  fseek(fs, sizeof(super_block) + sizeof(_block) * index, SEEK_SET);
  fread(&_block, sizeof(_block), 1, fs);
  
  return _block;
} 

void write_data_to_file(FILE *fs, entity file, char *data) {
  int counter = 0;

  for (size_t i = 0; i < COMMON_COUNT_BLOCKS; i++) {
    if (sb.free_blocks[i]) {
      char temp_data[SIZE_BLOCK] = {'\0'};
      strncpy(temp_data, &data[(SIZE_BLOCK - 1)*i], SIZE_BLOCK-1);
      block _block = create_block(fs, temp_data);
      
      
      for (size_t j = 0; j < COUNT_BLOCKS_IN_ENTITY; j++) {
        if (file.indexes[j] == -1) {
          file.indexes[j] = i;
          file.count_blocks++;
          break;
        }
      }
      
      write_block(fs, _block);

      if (++counter > (strlen(data) / COMMON_COUNT_BLOCKS)+1) {
        sb.entities[file.self_index] = file;
        return;
      }
    }
  }
}

entity get_entity(char title[128]) {
  for (size_t i = 0; i < COUNT_ENTITY; i++) {
    if (!strcmp(title, sb.entities[i].title)) return sb.entities[i];
  }
  entity _ = {.title=""};
  return _;
}

int main(int argc, char const *argv[]) {  
  char command[100];
  FILE *fs;
  
  char path[] = "file_system";

  if ((fs = fopen(path, "rb+")) == NULL) {
    printf("error");
    exit(1);
  }

  run_file_system(fs);
  
  // entity file = create_file(fs, "file1", 7);
  // write_data_to_file(fs, file, "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
  
  // entity _file = get_entity("file1");

  // for (size_t i = 0; i < _file.count_blocks; i++) {
  //   printf("block [%d]: %s\n", i, get_block(fs, _file.indexes[i]).data);
  // }

  char params[2][128];

  while (1) {
    printf("> ");
    // scanf("%[^\n]s", command);
    gets(command);

    if (!strcmp(command, "ls")) {
      printf("ls\n");
    } 
    else {
      char *ptr = strtok(command, " ");

      for (int i = 0; i < 2 && ptr; i++) {
        strcpy(params[i], ptr);
        ptr = strtok(NULL, " ");
      }

      if (!strcmp(params[0], "mkdir")) { // cf
        if (!strlen(params[1])) {
          printf("Name of file should not be empty!\n");
          continue;
        }
        create_file(fs, params[1], 7);
      }
      else if (!strcmp(params[0], "write")) {
        entity _file = get_entity(params[1]);
        if (!strlen(_file.title)) {
          printf("Name of file should not be empty!\n");
          continue;
        };

        printf("Write new line to file >> ");
        char line[1024];
        gets(line);
        write_data_to_file(fs, _file, line);
      }
      else if (!strcmp(params[0], "read")) {
        entity _file = get_entity(params[1]);
        for (size_t i = 0; i < _file.count_blocks; i++) {
          printf("block [%d]: %s\n", i, get_block(fs, _file.indexes[i]).data);
        }
      }
    }
  }
  
  sb.state = DISACTIVE;
  load_super_block(fs);
  fclose(fs);
  return 0;
}