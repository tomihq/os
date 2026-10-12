#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

static ssize_t nulo_read(struct file *filp, char __user *data, size_t s, loff_t *off) {
}

static ssize_t nulo_write(struct file *filp, const char __user *data, size_t s, loff_t *off) {
}

static struct file_operations nulo_operaciones = {
  .owner = THIS_MODULE,
};

static struct cdev nulo_device;
static dev_t major;
static struct class *nulo_class;

#define IO_OK 0
#define COUNT 1
#define DEVICE_NAME "nulo"

static int __init hello_init(void) {
  alloc_chrdev_region(&major, 0, COUNT, DEVICE_NAME);
  cdev_init(&nulo_device, &nulo_operaciones);
  cdev_add(&nulo_device, major, COUNT);


  nulo_class = class_create(THIS_MODULE, DEVICE_NAME);
  device_create(nulo_class, NULL, major, NULL, DEVICE_NAME);

  return IO_OK; 
}

static void __exit hello_exit(void) {
  device_destroy(nulo_class, major);
  class_destroy(nulo_class);
  cdev_del(&nulo_device);
  unregister_chrdev_region(major, COUNT);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("La banda de SO");
MODULE_DESCRIPTION("Una suerte de '/dev/null'");