#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <stdbool.h>

#define WR_VALUE _IOW('a','a',struct message*)
#define MAX_PCI_DEV_COUNT 64
#define COMMAND_PAGE "page"
#define COMMAND_PSI "pci_dev"

struct pci_dev_info {
    unsigned short device[MAX_PCI_DEV_COUNT];
    unsigned short vendor[MAX_PCI_DEV_COUNT];
    int pci_dev_count;
};

struct pg {
    bool status;
    unsigned long int virtualAddress;
    unsigned long int physicalAddress;
    int configPgtableLevels;
    long int pageSize;
    unsigned int flags;
    unsigned int refcount;

};

struct message {
    struct pci_dev_info *pdi;
    struct pg *page;
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Not enough arguments. Enter name of structure! Baka!\n");
        return 0;
    }
    struct message msg;
    struct pci_dev_info pdi;
    struct pg page;
    msg.pdi = &pdi;
    msg.page = &page;

    int fd;
        printf("\nOpening Driver\n");
        fd = open("/dev/etx_device", O_RDWR);
        if (fd < 0) {
            printf("Cannot open device file...\n");
            return 0;
        }


    printf("Writing data to Driver\n");
    ioctl(fd, WR_VALUE, (struct message *) &msg);

    if (strcmp(argv[1], COMMAND_PSI) == 0) {
        printf("pci found %d devices:\n", msg.pdi->pci_dev_count);
        for (int i = 0; i < msg.pdi->pci_dev_count; i++) {
            printf("\tpci found device = %d, vendor = %d\n", msg.pdi->device[i], msg.pdi->vendor[i]);
        }
    } else if (strcmp(argv[1], COMMAND_PAGE) == 0) {
        if (msg.page->status) {


            printf("Page virtual adress: 0x%lx, \n", msg.page->virtualAddress);

            printf("Page physical adress: 0x%lx, \n", msg.page->physicalAddress);
            printf("CONFIG_PGTABLE_LEVELS: %d, \n", msg.page->configPgtableLevels);
            printf("PAGE_SIZE: %ld, \n", msg.page->pageSize);
            printf("flags: %u l, \n", msg.page->flags);
            printf("refcount: %u \n", msg.page->refcount);
        } else {
            printf("Can not read Page... Yare-Yare daze...\n");
        }
    }
    else
    {
        printf("\nBaka! ");
        printf(argv[1]);
        printf(" Unsuitable argument\n");
    }
    printf("\nClosing Driver\n");
    close(fd);
}
