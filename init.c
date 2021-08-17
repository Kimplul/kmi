
Symtab for file arch/riscv/init/head.S at 0x55d5c1168a50
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: asm

Line table:

 line 6 at 0x83800000	(stmt)
 line 8 at 0x83800004	(stmt)
 line 10 at 0x83800006	(stmt)
 line 0 at 0x8380000a	(stmt)

Blockvector:

block #000, object at 0x55d5c1168ae0, 0 syms/buckets in 0x83800000..0x8380000a
  block #001, object at 0x55d5c1168a80 under 0x55d5c1168ae0, 0 syms/buckets in 0x83800000..0x8380000a


Symtab for file arch/riscv/init/init.c at 0x55d5c113f9f0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector:

block #000, object at 0x55d5c1168880, 0 syms/buckets in 0x83800000..0x83800000
  block #001, object at 0x55d5c11687c0 under 0x55d5c1168880, 4 syms/buckets in 0x83800000..0x83800000
   typedef char pagemask_t; 
   typedef long unsigned int long unsigned int; 
   typedef long double long double; 
   struct megapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct kilopages kilo[512];
   };

   struct mm_pagearr_info {
       short int tera_num;
       struct terapages *tera_top;
       short int giga_num;
       struct gigapages *giga_top;
       short int mega_num;
       struct megapages *mega_top;
       short int kilo_num;
       struct kilopages *kilo_top;
   };

   struct page_ret {
       size_t left;
       size_t num;
       void *top;
   };

   typedef long int long int; 
   typedef long unsigned int size_t; 
   typedef int int; 
   typedef long long int long long int; 
   typedef char char; 
   typedef char lockbit_t; 
   struct kilopages {
       pagemask_t page[64];
   };

   struct gigapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct megapages mega[512];
   };

   struct terapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct gigapages giga[512];
   };

   typedef short int short int; 

Compunit user: 0x55d5c115e2e0

Symtab for file /usr/lib/gcc/riscv64-unknown-elf/8.3.0/include/stddef.h at 0x55d5c113f990
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file arch/riscv/include/pages.h at 0x55d5c113f9c0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file include/apos/init.h at 0x55d5c113fa20
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file include/apos/compiler_attributes.h at 0x55d5c113fa50
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file include/apos/sizes.h at 0x55d5c113fa80
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file arch/riscv/include/vmap.h at 0x55d5c113fab0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/init/init.c


Symtab for file arch/riscv/main.c at 0x55d5c115e3f0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector:

block #000, object at 0x55d5c113f860, 0 syms/buckets in 0x83800000..0x83800000
  block #001, object at 0x55d5c113f7a0 under 0x55d5c113f860, 3 syms/buckets in 0x83800000..0x83800000
   struct kilopages {
       pagemask_t page[64];
   };

   struct gigapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct megapages mega[512];
   };

   struct terapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct gigapages giga[512];
   };

   typedef char lockbit_t; 
   struct megapages {
       lockbit_t lockbit[64];
       pagemask_t empty[64];
       pagemask_t full[64];
       struct kilopages kilo[512];
   };

   struct mm_pagearr_info {
       short int tera_num;
       struct terapages *tera_top;
       short int giga_num;
       struct gigapages *giga_top;
       short int mega_num;
       struct megapages *mega_top;
       short int kilo_num;
       struct kilopages *kilo_top;
   };

   typedef short int short int; 
   typedef char pagemask_t; 
   typedef char char; 
   typedef long unsigned int long unsigned int; 

Compunit user: 0x55d5c115e2e0

Symtab for file arch/riscv/include/pages.h at 0x55d5c115e3c0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/main.c


Symtab for file include/apos/main.h at 0x55d5c115e420
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/main.c


Symtab for file include/apos/compiler_attributes.h at 0x55d5c115e450
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector same as owning compunit: arch/riscv/main.c


Symtab for file <artificial> at 0x55d5c115e2e0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Blockvector:

block #000, object at 0x55d5c115dc10, 2 syms/buckets in 0x8380000a..0x8380000a
 void init(void *, void *, void *, void *, void *); block object 0x55d5c1158ff0, 0x83800160..0x838002f4 section .init
 void init_enter_vm(long unsigned int *); block object 0x55d5c115c820, 0x83800144..0x83800160
 long unsigned int init_align_down(size_t, size_t); block object 0x55d5c115cf00, 0x838000e4..0x83800116
 void init_zero(void *, size_t); block object 0x55d5c115da60, 0x8380000a..0x83800046
 void main(struct mm_pagearr_info *); block object 0x55d5c1159400, 0x43800000..0x4380000c section .init
 void call_main(); block object 0x55d5c11595f0, 0x4380000c..0x43800018 section .init
 long unsigned int init_align_up(size_t, size_t); block object 0x55d5c115cb50, 0x83800116..0x83800144
 struct page_ret {
     size_t left;
     size_t num;
     void *top;
 } init_pages(size_t, void *, size_t, size_t); block object 0x55d5c115d500, 0x83800046..0x838000e4
  block #001, object at 0x55d5c115db20 under 0x55d5c115dc10, 0 syms/buckets in 0x8380000a..0x8380000a
    block #002, object at 0x55d5c1159400 under 0x55d5c115db20, 1 syms/buckets in 0x43800000..0x4380000c, function main
     struct mm_pagearr_info {
         short int tera_num;
         struct terapages *tera_top;
         short int giga_num;
         struct gigapages *giga_top;
         short int mega_num;
         struct megapages *mega_top;
         short int kilo_num;
         struct kilopages *kilo_top;
     } *pageinfo; computed at runtime
    block #003, object at 0x55d5c11595f0 under 0x55d5c115db20, 0 syms/buckets in 0x4380000c..0x43800018, function call_main
    block #004, object at 0x55d5c115da60 under 0x55d5c115db20, 3 syms/buckets in 0x8380000a..0x83800046, function init_zero
     void *ptr; computed at runtime
     long unsigned int bytes; computed at runtime
     char *byte_ptr; computed at runtime
    block #005, object at 0x55d5c115d500 under 0x55d5c115db20, 7 syms/buckets in 0x83800046..0x838000e4, function init_pages
     long unsigned int ram_size; computed at runtime
     void *ram_top; computed at runtime
     long unsigned int page_size; computed at runtime
     long unsigned int struct_size; computed at runtime
     long unsigned int num_pages; computed at runtime
     long unsigned int size_pages; computed at runtime
     struct page_ret {
         size_t left;
         size_t num;
         void *top;
     } pageret; computed at runtime
    block #006, object at 0x55d5c115cf00 under 0x55d5c115db20, 3 syms/buckets in 0x838000e4..0x83800116, function init_align_down
     long unsigned int num; computed at runtime
     long unsigned int a; computed at runtime
     long unsigned int rem; computed at runtime
    block #007, object at 0x55d5c115cb50 under 0x55d5c115db20, 2 syms/buckets in 0x83800116..0x83800144, function init_align_up
     long unsigned int num; computed at runtime
     long unsigned int a; computed at runtime
    block #008, object at 0x55d5c115c820 under 0x55d5c115db20, 1 syms/buckets in 0x83800144..0x83800160, function init_enter_vm
     long unsigned int *root_page; computed at runtime
    block #009, object at 0x55d5c1158ff0 under 0x55d5c115db20, 9 syms/buckets in 0x83800160..0x838002f4, function init
     void *ram_base; computed at runtime
     void *ram_top; computed at runtime
     void *initrd; computed at runtime
     void *fdt; computed at runtime
     void *boot; computed at runtime
     long unsigned int ram_size; computed at runtime
     struct mm_pagearr_info {
         short int tera_num;
         struct terapages *tera_top;
         short int giga_num;
         struct gigapages *giga_top;
         short int mega_num;
         struct megapages *mega_top;
         short int kilo_num;
         struct kilopages *kilo_top;
     } pagearr; computed at runtime
     struct page_ret {
         size_t left;
         size_t num;
         void *top;
     } pageret; computed at runtime
     long unsigned int *root_page; computed at runtime

Compunit include: 0x55d5c115e3f0
Compunit include: 0x55d5c113f9f0

Symtab for file arch/riscv/main.c at 0x55d5c11185d0
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Line table:

 line 5 at 0x43800000	(stmt)
 line 0 at 0x4380000c	(stmt)
 line 0 at 0x4380000c	(stmt)
 line 0 at 0x8380000a	(stmt)

Blockvector same as owning compunit: <artificial>


Symtab for file arch/riscv/init/init.c at 0x55d5c1118600
Compilation directory is /home/kimi/Documents/Projects/aposkern
Read from object file /home/kimi/Documents/Projects/aposkern/kernel.debug (0x55d5c11488f0)
Language: c

Line table:

 line 63 at 0x4380000c	(stmt)
 line 64 at 0x43800012	(stmt)
 line 0 at 0x43800018	(stmt)
 line 16 at 0x8380000a	(stmt)
 line 16 at 0x8380000a	(stmt)
 line 17 at 0x83800018	(stmt)
 line 24 at 0x83800022	(stmt)
 line 24 at 0x8380002e	(stmt)
 line 23 at 0x83800032	(stmt)
 line 23 at 0x8380003e	(stmt)
 line 25 at 0x83800040	(stmt)
 line 29 at 0x83800046	(stmt)
 line 30 at 0x83800062	(stmt)
 line 31 at 0x83800072	(stmt)
 line 34 at 0x83800082	(stmt)
 line 34 at 0x83800090	(stmt)
 line 35 at 0x83800094	(stmt)
 line 36 at 0x8380009c	(stmt)
 line 36 at 0x838000a8	(stmt)
 line 36 at 0x838000b0	(stmt)
 line 38 at 0x838000b4	(stmt)
 line 40 at 0x838000c2	(stmt)
 line 41 at 0x838000d8	(stmt)
 line 44 at 0x838000e4	(stmt)
 line 45 at 0x838000f2	(stmt)
 line 46 at 0x83800102	(stmt)
 line 47 at 0x8380010e	(stmt)
 line 50 at 0x83800116	(stmt)
 line 51 at 0x83800126	(stmt)
 line 51 at 0x83800134	(stmt)
 line 52 at 0x8380013a	(stmt)
 line 55 at 0x83800144	(stmt)
 line 56 at 0x8380014e	(stmt)
 line 60 at 0x8380015a	(stmt)
 line 67 at 0x83800160	(stmt)
 line 68 at 0x8380017c	(stmt)
 line 68 at 0x83800188	(stmt)
 line 74 at 0x8380018c	(stmt)
 line 76 at 0x838001ac	(stmt)
 line 76 at 0x838001b0	(stmt)
 line 77 at 0x838001bc	(stmt)
 line 77 at 0x838001c0	(stmt)
 line 79 at 0x838001c4	(stmt)
 line 81 at 0x838001f8	(stmt)
 line 81 at 0x838001fc	(stmt)
 line 82 at 0x83800208	(stmt)
 line 82 at 0x8380020c	(stmt)
 line 84 at 0x83800210	(stmt)
 line 86 at 0x83800242	(stmt)
 line 86 at 0x83800246	(stmt)
 line 87 at 0x83800252	(stmt)
 line 87 at 0x83800256	(stmt)
 line 89 at 0x8380025a	(stmt)
 line 91 at 0x8380028a	(stmt)
 line 91 at 0x8380028e	(stmt)
 line 92 at 0x8380029a	(stmt)
 line 92 at 0x8380029e	(stmt)
 line 95 at 0x838002a2	(stmt)
 line 95 at 0x838002a6	(stmt)
 line 95 at 0x838002b0	(stmt)
 line 94 at 0x838002b4	(stmt)
 line 100 at 0x838002b8	(stmt)
 line 100 at 0x838002c0	(stmt)
 line 100 at 0x838002c4	(stmt)
 line 100 at 0x838002ce	(stmt)
 line 100 at 0x838002d2	(stmt)
 line 104 at 0x838002d4	(stmt)
 line 107 at 0x838002dc	(stmt)
 line 110 at 0x838002e4	(stmt)
 line 111 at 0x838002ec	(stmt)
 line 0 at 0x838002f4	(stmt)

Blockvector same as owning compunit: <artificial>

