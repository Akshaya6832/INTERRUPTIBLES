#include <errno.h>
int trinetra_spi_interface_init(void) { return 0; }
int trinetra_spi_interface_read(void *buffer, unsigned size)
{
    (void)buffer; (void)size; errno = ENOSYS; return -1;
}
