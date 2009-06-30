#include "cbob_digital.h"
#include "cbob_spi.h"
#include "cbob_cmd.h"

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/semaphore.h>
#include <asm/uaccess.h>

static int cbob_digital_major = CBOB_DIGITAL_MAJOR;

/* File Ops */

static ssize_t cbob_digital_read(struct file *file, char *buf, size_t count, loff_t *ppos);
static ssize_t cbob_digital_write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static int     cbob_digital_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
static int     cbob_digital_open(struct inode *inode, struct file *file);
static int     cbob_digital_release(struct inode *inode, struct file *file);

static struct file_operations cbob_digital_fops = {
	owner:   THIS_MODULE,
	open:    cbob_digital_open,
	release: cbob_digital_release,
	read:    cbob_digital_read,
	write:   cbob_digital_write,
	ioctl:   cbob_digital_ioctl
};

static int cbob_digital_open(struct inode *inode, struct file *file)
{
  struct digital_port *digital;
  
  digital = kmalloc(sizeof(struct digital_port), GFP_KERNEL);
  
  if(digital == 0)
    return -ENOMEM;
    
  digital->port = iminor(inode);
  
  file->private_data = digital;
  
  
  return 0;
}

static int cbob_digital_release(struct inode *inode, struct file *file)
{
  kfree(file->private_data);
  return 0;
}

static ssize_t cbob_digital_read(struct file *file, char *buf, size_t count, loff_t *ppos) 
{
  struct digital_port *digital = file->private_data;
  short data;
  int error;
  
  if((error = cbob_spi_message(CBOB_CMD_DIGITAL_READ, &(digital->port), 1, &data, 1)) < 0)
    return error;
  
  if(count > 2)
    count = 2;
  
  copy_to_user(buf, (char*)&data, count);
  
  return count;

}

static ssize_t cbob_digital_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	struct digital_port *digital = file->private_data;
	char kbuf;
	short data;
	int error;
	
	if(count == 0)
		return 0;
	
	copy_from_user(&kbuf, buf, 1);
	data = kbuf;
	
	if((error = cbob_spi_message(CBOB_CMD_DIGITAL_WRITE, &(digital->port), 1, &data, 1)) < 0)
		return error;
	
  return 1;
}

static int cbob_digital_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  return 0;
}

/* init and exit */
int cbob_digital_init(void)
{
  int error;
  
  error = register_chrdev(cbob_digital_major, CBOB_DIGITAL_NAME, &cbob_digital_fops);
  
  if(error < 0) {
    printk(KERN_ALERT "Failed to register cbob_digital char device with error: %d\n", error);
    return error;
  }
  
  return 0;
}

void cbob_digital_exit(void)
{
  int error;
  
  error = unregister_chrdev(cbob_digital_major, CBOB_DIGITAL_NAME);
  if(error < 0) {
    printk(KERN_ALERT "Failed to unregister cbob_digital char device with error: %d\n", error);
  }
}