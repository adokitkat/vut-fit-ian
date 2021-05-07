#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <elf.h>
#include <libelf.h>
#include <gelf.h>

struct Section {
  char* name;
  size_t offset;
  size_t size;
  struct Section* next;
};

const char* typeLookupTable(int i) 
{
  switch (i) 
  {
    case PT_NULL:         return "NULL";
    case PT_LOAD:         return "LOAD";
    case PT_DYNAMIC:      return "DYNAMIC";
    case PT_INTERP:       return "INTERP";
    case PT_NOTE:         return "NOTE";
    case PT_SHLIB:        return "SHLIB";
    case PT_PHDR:         return "PHDR";
    case PT_TLS:          return "TLS";
    case PT_NUM:          return "NUM";
    case PT_LOOS:         return "LOOS";
    case PT_GNU_EH_FRAME: return "GNU_EH_FRAME";
    case PT_GNU_STACK:    return "GNU_STACK";
    case PT_GNU_RELRO:    return "GNU_RELRO";
    case PT_GNU_PROPERTY: return "GNU_PROPERTY";
    //case PT_LOSUNW: return "LOSUNW";
    case PT_SUNWBSS:      return "SUNWBSS";
    case PT_SUNWSTACK:    return "SUNWSTACK";
    //case PT_HISUNW: return "HISUNW";
    case PT_HIOS:         return "HIOS";
    case PT_LOPROC:       return "LOPROC";
    case PT_HIPROC:       return "HIPROC";
    default:              return "";
  }
}

const char* permissionLookupTable(int i) 
{
  switch (i) 
  {
    case 0: return "---";
    case 1: return "--X";
    case 2: return "-W-";
    case 3: return "-WX";
    case 4: return "R--";
    case 5: return "R-X";
    case 6: return "RW-";
    case 7: return "RWX";
    case PF_MASKOS: return "MASKOS";
    case PF_MASKPROC: return "MASKPROC";
    default: return "";
  }
}

// Function for help text
void showHelp()
{
  printf("Usage:\n"
  "  ./ian-proj1 <file path>\n\n"
  "Shows ELF info\n"
  );
}

void graceful_exit(Elf* e, int fd) {
  if (e != NULL)
    { elf_end(e); }
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    showHelp();
    return 1;
  } else if (argc > 2) {
    fprintf(stderr, "Too many arguments\n");
    return 1;
  }

  char* path = argv[1];

  if ((strcmp(path, "-h") == 0) || (strcmp(path, "--help") == 0) || (strcmp(path, "-?") == 0)) {
    showHelp();
    return 0;
  }

  int fd = open(path, O_RDONLY, 0);
  if (fd < 0) {
    fprintf(stderr, "Wrong path\n");
    return 1;
  }

  if (elf_version(EV_CURRENT) == EV_NONE ) { 
    fprintf(stderr, "ELF library initialization failed : %s\n" , elf_errmsg(-1));
    return 1;
  }

  Elf* e = elf_begin(fd, ELF_C_READ, NULL);
  if (e == NULL) {
    fprintf(stderr, "Cound not read the file\n");
    fprintf(stderr, "elf_begin() failed: %s\n", elf_errmsg(-1));
    graceful_exit(e, fd);
    return 1;
  }

  Elf_Kind ek = elf_kind(e);
  if (ek != ELF_K_ELF) {
    fprintf(stderr, "Not an ELF file\n");
    graceful_exit(e, fd);
    return 1;
  }

  int elf_class = gelf_getclass(e); // ELFCLASS32 or ELFCLASS64
  if (elf_class == ELFCLASSNONE) {
    fprintf(stderr, "Invalid ELF class\n");
    graceful_exit(e, fd);
    return 1;
  }

  if (elf_class == ELFCLASS32) {
    fprintf(stderr, "32bit ELF file is not supported right now\n");
    graceful_exit(e, fd);
    return 1;
  }

  GElf_Ehdr e_header;
  if (gelf_getehdr(e, &e_header) == NULL) {
    fprintf(stderr, "Cound not get an ELF file header\n");
    fprintf(stderr, "gelf_getehdr() failed: %s\n", elf_errmsg(-1));
    graceful_exit(e, fd);
    return 1;
  }

  size_t section_index;

  if (elf_getshdrstrndx(e, &section_index) != 0) {
    fprintf(stderr, "Cound not retrieve a section index from the ELF file\n");
    fprintf(stderr, "elf_getshdrstrndx() failed: %s\n", elf_errmsg(-1));
    graceful_exit(e, fd);
    return 1;
  }

  GElf_Shdr section_header;
  Elf_Scn* section = NULL;
  char* section_name;

  struct Section *head = NULL,
                **tail = &head;

  while ((section = elf_nextscn(e, section)) != NULL) {

    if (gelf_getshdr(section , &section_header) != &section_header) {
      fprintf(stderr, "gelf_getshdr() failed: %s\n", elf_errmsg(-1));
      graceful_exit(e, fd);
      return 1;
    }

    section_name = elf_strptr(e, section_index, section_header.sh_name);
    if (section_name == NULL) {
      fprintf(stderr, "elf_strptr() failed: %s\n", elf_errmsg(-1));
      graceful_exit(e, fd);
      return 1;
    }

    if (section_header.sh_addr == 0) { continue; }
    
    *tail = malloc(sizeof(**tail));
    (*tail)->name = malloc(sizeof(section_name));
    strcpy((*tail)->name, section_name);
    (*tail)->offset = section_header.sh_offset;
    (*tail)->size = section_header.sh_size;
    tail = &(*tail)->next;
  }

  size_t n_segments;
  if (elf_getphdrnum(e, &n_segments) != 0) {
    fprintf(stderr, "elf_getphdrnum() failed: %s\n", elf_errmsg(-1));
    graceful_exit(e, fd);
    return 1;
  }

  if (n_segments > 0)
    { printf("Segment\tType\t\tPerm\tSections\n"); }

  GElf_Phdr program_header;
  const char* type = NULL;

  for (int i = 0; i < n_segments; i++)
  {
    if (gelf_getphdr(e, i, &program_header) != &program_header) {
      fprintf(stderr, "gelf_getphdr() failed: %s\n", elf_errmsg(-1));
      graceful_exit(e, fd);
      return 1;
    }

    type = typeLookupTable(program_header.p_type);
    printf("%02d\t%s\t", i, type);
    
    if (strlen(type) <= 7)
      { printf("\t"); }
    
    printf("%s\t", permissionLookupTable(program_header.p_flags));
    
    for (struct Section *ptr = head; ptr; ptr = ptr->next)
    {
      if (ptr->offset >= program_header.p_offset && ptr->offset < (program_header.p_offset + program_header.p_memsz) && ptr->size != 0)
      { 
        printf("%s ", ptr->name);
      }
    }
    printf("\n");
  }

  // Free allocated memory
  struct Section* tmp;
  while (head != NULL) {
    tmp = head;
    head = head->next;
    free(tmp->name);
    free(tmp);
  }

  graceful_exit(e, fd);
  return 0;
}