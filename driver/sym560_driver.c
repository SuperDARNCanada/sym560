/* File : 	sym560_driver.c
 * Author: 	Bradley Krug
 * Modified:	Feb 21 (added some comments)
 * Description:	This is a driver for the Symmetricom 560-5908-U (GPS-PCI-2U) card.  It was 
 *		written using principles found in the online book Linux Device Drivers (3rd Ed).
 */

#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h> 
/* fs.h is for the alloc_chrdev_region
 * types.h is for the dev_t data structure
 * kdev_t.h used for the MAJOR(dev_t dev) macro
 * cdev.h needed for cdev_init, cdev_add and cdev_del
 * asm/io.h needed for the ioread / iowrite functions 
 * asm/uaccess.h needed for copy_to/from_user
 * linux/ioctl.h needed for the IOC macros (which are actually in asm/ioctl.h)  
 * asm/atomic.h needed for the atomic_t variable type
 */ 

/*****************************************************************************/
/* GLOBAL VARIABLES */

/* The following values come from page 25 of the 560-5908 User Guide */
#define SYM560_VENDOR_ID 	0x10b5
#define SYM560_DEVICE_ID 	0x9050
#define SYM560_SUBVENDOR_ID	0x12DA
#define SYM560_SUBSYSTEM_ID	0x5908

/* Register offsets */
/* See Symmetricom manual for details (Ch 3) */
#define REGOFF_INTCONT		0xF8	/*Interrupt and flag control (1 byte)*/
#define REGOFF_STIMECAP		0xFC	/*Software Time Capture (12 bytes)*/
#define REGOFF_HSTATUS		0xFE	/*Hardware Status (1 byte, part of STIMECAP)*/
#define REGOFF_ANTPOS		0x108	/*Antenna Hardware Position (16 bytes)*/
#define REGOFF_CONFIG1		0x118	/*Configuration #1 (4 bytes)*/
#define REGOFF_DIAGN		0x11C	/*Diagnostic (4 bytes)*/
#define REGOFF_TZO		0x120	/*Time Zone Offset (4 bytes)*/
#define REGOFF_PHCOMP		0x124	/*Phase Compensation (3 bytes)*/
#define REGOFF_FACTCAL		0x127	/*Factory Calibration (1 byte)*/
#define REGOFF_RATESYN		0x128	/*Rate Synthesizer (4 bytes)*/
#define REGOFF_CONFIG2		0x12C	/*Config #2 (4 bytes)*/
#define REGOFF_TIMECOMP		0x138	/*Time Config (8 bytes)*/
#define REGOFF_PRESETT		0x158	/*Preset Time (12 bytes)*/
#define REGOFF_PRESETP		0x164	/*Preset Position (16 bytes)*/
#define REGOFF_EVENTCAP		0x174	/*Event Time Capture (12 bytes)*/
#define REGOFF_SIGSTR		0x198	/*Satellite Signal Strength (24 bytes)*/
#define REGOFF_SIGSTRFLAG	0x1B0	/*Satellite signal strength update status (1 byte)*/
#define REGOFF_IRIGAM		0x1B4	/*IRIG AM AGC Delays (4 bytes)*/
#define REGOFF_VER		0x1BC	/*PCI card software version (4 bytes)*/




#define MAX_NUM_DEVICES		1

/* magic number chosen to avoid conflicts
 *  (see /usr/src/linux/Documentation/ioctl-number.txt) */
#define SYM560_IOC_MAGIC	0xF8

/* IOCTL Commands */
#define SYM560_EVENT_CAPTURE	_IOR(SYM560_IOC_MAGIC, 0, long long int)
#define SYM560_SIMPLETEST	_IO(SYM560_IOC_MAGIC, 1)
#define SYM560_CHECKSIGNAL	_IO(SYM560_IOC_MAGIC, 2)
#define SYM560_CHECK_INTCSR	_IO(SYM560_IOC_MAGIC, 3)

/* Queue for waiting for event interrupts */
static DECLARE_WAIT_QUEUE_HEAD(event_queue);
static int event_flag = 0;

/* MAJOR and MINOR device numbers,
 * MAJOR is dynamically assigned with the alloc_chrdev_region function
 * MINOR should be 0 */ 
int SYM560_MAJOR = 0;
int SYM560_MINOR = 0;

/* OPEN_CNT keeps track of how many times the device file is opened.  
 * atomic_t variable types ensure that two different processes can't be 
 * checking the variable at the same time.  
 */
atomic_t OPEN_CNT = ATOMIC_INIT(-1);
/*****************************************************************************/


/*****************************************************************************/\
/* STRUCTS */

/* Create the struct of devices that this driver will support */
static struct pci_device_id sym560_ids[] = {
	{ SYM560_VENDOR_ID, SYM560_DEVICE_ID, SYM560_SUBVENDOR_ID, SYM560_SUBSYSTEM_ID, 0, 0, 0},
	{0, }
};

/* peripheral descriptor used to keep track of memory allocation */
struct sym560_descriptor {
	unsigned long memstart;	/* address from Base address register 2 */
	unsigned long memlen;	/* Total size of the memory space */
	void *vmemaddr;		/* mapped virtual memory address */
	unsigned long lcrstart; /* address from Base address register 0 */
	unsigned long lcrlen;	/* total size of lcr (local config registers) */
				/* see ch 3.3.2 of sym560 manual */
	void *vlcraddr;		/* mapped virtual mem address for local config reg */
	u8 irq;			/* Interrupt ReQuest number */
	u32 evdata[3]; 	/* struct to hold the most current event data */
	struct cdev mycdev;	/* Char device structure */
}sym560;
/*****************************************************************************/


/* Export (using the MODULE_DEVICE_TABLE macro) the struct to user space */
/* should this be put in a function??? */
MODULE_DEVICE_TABLE(pci, sym560_ids);


/*****************************************************************************/
/* INTERRUPT HANDLER */
/*****************************************************************************/
/* pt_regs no longer passed to event handler. Speed increase is the reasoning */
/*irqreturn_t sym560_event_handler(int irq, void *dev_id, struct pt_regs *regs)*/
irqreturn_t sym560_event_handler(int irq, void *dev_id)
{
	struct sym560_descriptor *dev;
	u8 data_8;
	dev = dev_id;
	data_8 = ioread8(dev->vmemaddr + 0xFE);
	
	/* read in evdata */
	dev->evdata[0] = ioread32(dev->vmemaddr + REGOFF_EVENTCAP );
	dev->evdata[1]= ioread32(dev->vmemaddr + REGOFF_EVENTCAP + 4);
	dev->evdata[2]= ioread32(dev->vmemaddr + REGOFF_EVENTCAP + 8);
	
	data_8 = ioread8(dev->vmemaddr + 0xF8);
	/* the following OR will set a 1 to all the clear bits while leaving 
	 * the other bits untouched */
	data_8 = data_8 | 0x47;
	iowrite8(data_8, dev->vmemaddr + 0xF8);
	
	event_flag = 1;
	wake_up_interruptible(&event_queue);
	
	return IRQ_HANDLED;
}


/*****************************************************************************/
/* FILE OPERATIONS */
/*****************************************************************************/

/*****************************************************************************/
/* NAME: 	sym560_llseek
 *
 * ARGUMENTS:	file *filp - The file that this llseek is being performed on.
 * 		loff_t off - The new file position
 * 		int whence - Can normally be either 0, 1, or 2 where
 * 				0 : offset from beginning of file
 * 				1 : offset from current position
 * 				2 : offset from end of file
 * 		For this driver, only 0 is allowed.
 * 
 * RETURNS:	The new file position
 * 
 * DESCRIPTION: Sets the current file position so that the following read/write
 * 		will be done at the appropriate offset. This operation is called
 * 		when a user application attempts to change the current position
 * 		in the file using the user space function fseek.
 */ 		 
static loff_t sym560_llseek(struct file *filp, loff_t off, int whence)
{

	struct sym560_descriptor *dev; /* dev will contain device info */
	dev = filp->private_data; 
	
	if (whence != 0 || off > dev->memlen || off < 0 )
		return -EINVAL;
	
	/* filp->pos is set so that sym560_read/write can make use the offset */
	filp->f_pos = off;
	
	/* DEBUGGING MESSAGE */
	/*printk(KERN_DEBUG "Offset changed to %lx\n", (long int)(off));*/

	return off;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_open
 *
 * ARGUMENTS:	inode *inode - inode that the device file belongs to
 * 		file *filp   - file structure that the device file belongs to
 *		 
 * RETURNS:	0 for success
 * 
 * DESCRIPTION: This is a file operation that gets called whenever a user space
 * 		application tries to open (using fopen) the device file.  For 
 * 		this driver it simply prints a message.  However, it may be a 
 * 		good idea to do some checks to make sure the file is not already
 * 		open.
 */ 		 
static int sym560_open(struct inode *my_inode, struct file *filp)
{
	int ret;

	struct sym560_descriptor *dev; /* dev will contain device info */
	/*pg 12 ch 3 of rubini for explanation of following command */
	dev = container_of(my_inode->i_cdev, struct sym560_descriptor, mycdev);
	
	/* store the dev struct in filp so it can be used in other functions */
	filp->private_data = dev;

	/* atomic_inc_and_test returns true if OPEN_CNT is 0 after having been
	 * incremented. */
	if (atomic_inc_and_test(&OPEN_CNT))
	{
		/* If this is true than the device file is not currently opened
		 * by any other process. */
		
		/* request IRQ */
		ret = request_irq(dev->irq, sym560_event_handler, IRQF_SHARED, "sym560_pci_card", dev);
		if (ret != 0)
		{
			printk(KERN_ERR "Could not register irq #%d\n", dev->irq);
			return ret;
		}
		enable_irq(dev->irq);
		printk(KERN_DEBUG "Just requested_irq: %d\n",dev->irq);
		atomic_inc(&OPEN_CNT);
	}
	printk(KERN_DEBUG "\nThe sym560 device has been opened\n");
	
	return 0;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_release
 *
 * ARGUMENTS:	inode *inode - inode that the device file belongs to
 * 		file *filp   - file structure that the device file belongs to
 *		 
 * RETURNS:	0 for success
 * 
 * DESCRIPTION: This is a file operation that gets called whenever a user space
 * 		application tries to close (using fclose) the device file.  For 
 * 		this driver it simply prints a message.  However, it should undo
 * 		anything that is done in sym560_open.
 */ 		 
static int sym560_release(struct inode *my_inode, struct file *filp)
{
	/* decrement the OPEN_CNT variable */
	/* if last instance then free the irq */

	struct sym560_descriptor *dev; /* dev will contain device info */
	dev = filp->private_data; 
	/* clear the status flag */
	

	if ((atomic_dec_and_test(&OPEN_CNT)))
	{
		/* this is the last file to be closed */
		/* free irq */
		free_irq(dev->irq, dev);
		atomic_dec(&OPEN_CNT);	
	}
	printk(KERN_INFO "\nThe sym560 device has been closed\n");
	return 0;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_read
 *
 * ARGUMENTS:	file *filp   - file structure that the device file belongs to
 *		char __user *buf - user space buffer
 *		size_t count - size of requested data transfer
 *		loff_t *f_pos - file position (as set by sym_560 llseek above)
 *
 * RETURNS:	The number of bytes successfully read.
 * 		Negative number if unsuccessful.
 * 
 * DESCRIPTION: Reads the pci card from offset and passes the value to the user.
 * 		Does this for 'count' bytes.  Called whenever user application
 * 		calls the read() function.
 */
static ssize_t sym560_read(struct file *filp, char __user *buf, size_t count,
		loff_t *f_pos)
{
	void *offset_address, *kern_buf;
	unsigned int data_read;
	int ret;
	
	struct sym560_descriptor *dev; 	/* dev will contain device info */

	/* retrieve dev structure which was stored in sym560_open */
	dev = filp->private_data;
	
	/* DEBUGGING MESSAGE */
	/*printk(KERN_DEBUG "\nSym560 is being read...\n");*/
	
	/* set the address that will be read */
	offset_address = dev->vmemaddr + *f_pos;

	/* read 1 byte */
	if (count == 1)
	{
		/* ioread will return 8 bits from the kernel address provided */
		data_read = ioread8(offset_address);
	}
	else if (count == 2)
	{
		data_read = ioread16(offset_address);
	}
	else if (count == 4)
	{
		data_read = ioread32(offset_address);
	}
	else
	{
		printk(KERN_INFO "Failed to read %lx bytes of data\n",count);
		return -1;
	}	
	
	/* point kernel buffer to the data */
	kern_buf = &data_read;
	/* DEBUGGINB MESSAGES */
	/*printk(KERN_DEBUG "    Address being read = %p\n", offset_address);
	printk(KERN_DEBUG "    1st Byte (in hex)  = %x\n", data_read);
	printk(KERN_DEBUG "    Total num of bytes = %d\n", count);*/
	
	/* copy_to_user will copy to buf from kern_buf for 1 byte of data */
	ret = copy_to_user(buf, kern_buf, count);
	if (ret != 0)
	{
		printk(KERN_ERR "%d byte(s) could not be transferred to user space\n", ret);
		return -EFAULT;
	}
	/* DEBUGGING MESSAGE */
	/*printk(KERN_DEBUG "Byte(s) successfully copied to user space\n");*/
	return count;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_write
 *
 * ARGUMENTS:	file *filp   - file structure that the device file belongs to
 *		const char __user *buf - user space buffer
 *		size_t count - size of requested data transfer
 *		loff_t *f_pos - file position (as set by sym_560 llseek above)
 *
 * RETURNS:	The number of bytes successfully written.
 * 
 * DESCRIPTION: Writes 'count' bytes to the pci card starting at 'offset'.  Called 
 * 		whenever a user application invokes the function fwrite().
 */
static ssize_t sym560_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	
	void *offset_address, *kern_buf; 
	u8 data_write8;
	u16 data_write16;
	u32 data_write32;
	int ret;
	
	struct sym560_descriptor *dev;
	dev = filp->private_data;	/* recover device info (such as current offset) */
	/* set the address that will be read */
	offset_address = dev->vmemaddr + *f_pos;

	if (count == 1)
	{
		/* DEBUGGING MESSAGE */
		/*printk(KERN_DEBUG "\nSym560 is being written...\n");*/
		
		kern_buf = &data_write8;
		/* get data from the user */
		ret = copy_from_user(kern_buf, buf, count);
		if (ret != 0)
		{
			printk(KERN_ERR "%d byte(s) could not be read from user space\n",ret);
			return -EFAULT;
		}

		/* DEBUGGING MESSAGE */
		/*printk(KERN_DEBUG "Writing 1 byte (0x%x) to address %p\n", data_write8, offset_address);*/
		iowrite8(data_write8, offset_address);
	}
	else if (count == 2)
	{
		kern_buf = &data_write16;
		ret = copy_from_user(kern_buf, buf, count);
		if (ret != 0)
		{
			printk(KERN_ERR "%d byte(s) could not be read from user space\n",ret);
			return -EFAULT;
		}
		/* DEBUGGING MESSAGE */
		/*printk(KERN_DEBUG "Writing 2 bytes (0x%x) to address %p\n", data_write16, offset_address);*/
		iowrite16(data_write16, offset_address);
	}
	else if (count == 4)
	{
		kern_buf = &data_write32;
		ret = copy_from_user(kern_buf, buf, count);
		if (ret != 0)
		{
			
			printk(KERN_ERR "%d byte(s) could not be read from user space\n",ret);
			return -EFAULT;
		}
		/* DEBUGGING MESSAGE */
		/*printk(KERN_DEBUG "Writing 4 byte (0x%x) to address %p\n", data_write32, offset_address);*/
		iowrite32(data_write32, offset_address);
	}
	else
	{
		printk(KERN_ERR "Cannot write %lx bytes of data\n",count);
		return -1;
	}	
	/* DEBUGGING MESSAGE */
	/*printk(KERN_DEBUG "Byte(s) successfully copied from user space\n");*/
	return count;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_ioctl
 *
 * ARGUMENTS:	
 *
 * RETURNS:	Dependant on specific command called 
 * 
 * DESCRIPTION: This function is called whenever a user program makes a call using the ioctl function.
 *		It's basically a switch statement that checks to see which command it should run
 *		The available commands (such as SYM560_EVENT_CAPTURE) are assigned a number (globally 
 *		near the top of this file) and these numbers are then printed out to the log messages
 *		whenever the module is loaded and the device is initialized via the sym560_probe function.
 *		These numbers don't change (as far as I can tell) so the device can be reloaded 
 *		and the numbers will be the same.  Ex: if a user application calls the command 
 *		ioctl(fd, 0x8008f800) then EVENT_CAPTURE is chosen.
*/
/*static int sym560_ioctl(struct inode *my_inode, struct file *filp, unsigned int cmd, unsigned long arg) */
static long sym560_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
//	lock_kernel();
	char tmpbuff[5];
	int ret;
	u32 data_32;
	struct sym560_descriptor *dev; /* dev will contain device info */
	/*pg 12 ch 3 of rubini for explanation of following command */
	dev = container_of(filp->f_path.dentry->d_inode->i_cdev, struct sym560_descriptor, mycdev);

	switch(cmd)
       	{
		/* The event capture io command used for timestamping */
		case SYM560_EVENT_CAPTURE:
			/* wait for the interrupt to occur */
			wait_event_interruptible(event_queue, event_flag != 0);	
			event_flag = 0;
			
			ret = copy_to_user((int __user *) arg, dev->evdata, 12);
			if (ret != 0)
			{	
				printk(KERN_ERR "%d byte(s) could not be transferred to user space\n", ret);
//				unlock_kernel();
				return -EFAULT;
			}
			break;
		/* Initial IO command used for debugging/testing purposes */
		case SYM560_SIMPLETEST:
			printk(KERN_DEBUG "\nSimpletest was called\n");
			break;
		/* Second IO command made to show that applications could be built directly into driver */
		case SYM560_CHECKSIGNAL:
			printk(KERN_DEBUG "\nCheckSignal was called\n");
			tmpbuff[0] = ioread8(dev->vmemaddr + REGOFF_SIGSTRFLAG);
			printk(KERN_DEBUG "\nSatellite A Info: %x\n",tmpbuff[0]);
			if(tmpbuff[0] != 0)
			{
				printk(KERN_DEBUG "SATELLITE STATUS IS CURRENTLY UNAVAILABLE OR UPDATING\n");
//				unlock_kernel();
				return -ENOTTY;
			}
			tmpbuff[1] = ioread32(dev->vmemaddr + REGOFF_SIGSTR);
			printk(KERN_DEBUG "\n Satellite A SV Number : %x\n", tmpbuff[1]);
			printk(KERN_DEBUG "\n Satellite signal level tenths: %x\n", tmpbuff[3]);
			printk(KERN_DEBUG "\n Satellite signal level tens : %x\n", tmpbuff[4]);
			
			ret = copy_to_user((int __user *) arg, tmpbuff, 5);
			break;

		/* Ensure interrupts are enabled on PLX9050 */
		case SYM560_CHECK_INTCSR:
			printk(KERN_DEBUG "\n\nChecking INTCSR Register\n");
			data_32 = ioread32(dev->vlcraddr + 0x4c);
			printk(KERN_DEBUG "\n INTCSR Register = %u (0x%x)\n",data_32,data_32);
			/* only worried about bottom two bytes */
			if ((data_32 & 0x000000FF) != 0x00000048)
			{
				data_32 = 0x48;	
				iowrite32(data_32, dev->vlcraddr + 0x4c);
				data_32 = ioread32(dev->vlcraddr + 0x4c);
				printk("Changed to %u (0x%x)\n",data_32,data_32);
			}
			break;
		default: /* command not allowed */
//			unlock_kernel();
			return -ENOTTY;
	}
	
	//unlock_kernel();
	return 0;


}


/* Declare the operations that the device file can use */
static struct file_operations sym560_fops = {
	.owner = 		THIS_MODULE,
	.llseek = 		sym560_llseek,
	.read = 		sym560_read,
	.write = 		sym560_write,
	.open =			sym560_open,
	.release = 		sym560_release,
	.unlocked_ioctl = 	sym560_ioctl,
};
/*****************************************************************************/
/* END OF FILE OPERATIONS */
/*****************************************************************************/



/*****************************************************************************/
/* NAME: 	sym560_get_info
 *
 * ARGUMENTS:	struct pci_dev *dev
 * 
 * RETURNS:	The revision # of the the card... this should be changed	
 * 
 * DESCRIPTION: Access the configuration space of the card and prints out the
 * 		Vendor ID, the Device ID and the Revision #.  This function was
 * 		made as a check while developing the driver. 	 
 */ 	
static unsigned char sym560_get_info(struct pci_dev *dev)
{
	u8 revision;
	u16 vendorid, deviceid;
	
	/* PCI_VENDOR_ID and others, are defined in pci_regs.h. */
	pci_read_config_word(dev, PCI_VENDOR_ID, &vendorid);
	pci_read_config_word(dev, PCI_DEVICE_ID, &deviceid);
	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	printk(KERN_INFO "Vendor ID= %x, Device ID= %x, Revision = %x\n", vendorid,deviceid, revision);
	return revision;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_probe
 *
 * ARGUMENTS:	struct pci_dev *dev
 * 		const struct pci_device_id *id
 * 
 * RETURNS:	0 if device is successfully probed
 * 		negative number otherwise (need to work on this)	
 * 
 * DESCRIPTION: This probe function is automatically called whenever a pci
 * 		card that this driver can support is detected (it only supports
 * 		the symmetricom 560 card). It performs various initializations.
 */
static int sym560_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	
	int ret;	/* used for error handling */
	/*struct that holds the major and minor device numbers*/
	dev_t devt = MKDEV(SYM560_MAJOR, SYM560_MINOR);
	
	/* struct used in memory management declared above */
	struct sym560_descriptor *sym560_p = &sym560;
	
	printk(KERN_INFO "\n**************************************************\n");
	printk(KERN_INFO "Probing the Symmetricom 560-590U (sym560) PCI card\n");
	printk(KERN_INFO "\n**************************************************\n");	

	/* ??? what exactly does this do? */
	pci_set_drvdata(dev, sym560_p);
	
	pci_enable_device(dev);
	
	sym560_get_info(dev);
	
	/* get Base Address Register 2 (as needed according to
	 * page 25 of the sym560 user manual) */
	sym560_p->memstart = pci_resource_start(dev, 2);
	sym560_p->memlen = pci_resource_len(dev, 2);
	
	/* get Base Address Register 0 (pg 26 of user manual) */
	sym560_p->lcrstart = pci_resource_start(dev, 0);
	sym560_p->lcrlen = pci_resource_len(dev, 0);
	
	
	printk(KERN_DEBUG "\nSym560 Memory Specs:\n");
	printk(KERN_DEBUG "    Base Address Register 2 = %lu\n", sym560_p->memstart);
	printk(KERN_DEBUG "    Length                  = %lu bytes\n", sym560_p->memlen);
	printk(KERN_DEBUG "    Base Address Register 0 = %lu\n", sym560_p->lcrstart);
	printk(KERN_DEBUG "    Length                  = %lu bytes\n", sym560_p->lcrlen);

	/* Now request the region (return value has no meaning)*/
	request_mem_region(sym560_p->memstart, sym560_p->memlen, "sym560");
	request_mem_region(sym560_p->lcrstart, sym560_p->lcrlen, "sym560lcr");

	/* Map it to virtual memory so Kernel can access it */
	sym560_p->vmemaddr = ioremap(sym560_p->memstart, sym560_p->memlen);
	sym560_p->vlcraddr = ioremap(sym560_p->lcrstart, sym560_p->lcrlen);
	
	printk(KERN_DEBUG "    Mapped Virutal Address  = %p\n", sym560_p->vmemaddr);
	printk(KERN_DEBUG "    Mapped LCR VirtAddress  = %p\n\n",sym560_p->vlcraddr);
	
	/* get interrupt number */
	/* apparently this is not the correct way of doing it */
	/* ret = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &(sym560_p->irq)); */
	/* instead ... */
	sym560_p->irq = dev->irq;
	
	printk(KERN_DEBUG "IRQ NUM = %d\n", sym560_p->irq);
	
	/* assign major/minor device numbers */
	/* This will cause it to show up in /proc/devices */
	ret = alloc_chrdev_region(&devt, SYM560_MINOR, 1, "symgps"); 
	
	if (ret != 0)
	{
		printk(KERN_ERR "could not allocate chrdev region\n");
		/* goto statement for cleaning up needed */
		return ret;
	}
	
	SYM560_MAJOR = MAJOR(devt);

	printk(KERN_DEBUG "Sym560 registered as:\n");
	printk(KERN_DEBUG "    Major ID = %d\n", SYM560_MAJOR);	
	printk(KERN_DEBUG "    Minor ID = %d\n", SYM560_MINOR);	

	/* next bit of code registers the char device */
	/* chapter 3 of O'Reilley Linux Device Drivers explains this */
	cdev_init(&(sym560_p->mycdev), &sym560_fops); 
	sym560_p->mycdev.owner = THIS_MODULE;
	sym560_p->mycdev.ops = &sym560_fops;
	ret = cdev_add(&(sym560_p->mycdev), devt, 1);
	if(ret)
	{
		printk(KERN_NOTICE "Error %d adding symgps\n",ret);
		return ret;
	}
	
	printk(KERN_DEBUG "\nValid ioctl command are as follows:\n");
	printk(KERN_DEBUG "event capture : %lx\n",SYM560_EVENT_CAPTURE);
	printk(KERN_DEBUG "simpletest : %x\n",SYM560_SIMPLETEST); 
	printk(KERN_DEBUG "Check Signal: %x\n", SYM560_CHECKSIGNAL);
	printk(KERN_DEBUG "Check INTCSR: %x\n", SYM560_CHECK_INTCSR);
	return 0;

	/* add goto statements here for dealing with errors */
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME: 	sym560_remove
 *
 * ARGUMENTS:	struct pci_dev *dev
 * 
 * RETURNS:	Nothing	
 * 
 * DESCRIPTION: This function is automatically called whenever the device is
 * 		being unregistered (rmmod).  Basically it should undo EVERYTHING
 * 		that was done in the probe function in reverse order.
 */ 		 
static void sym560_remove(struct pci_dev *dev)
{
	/* struct used to hold the memory locations */
	/* It was SET in probe and now we GET it here */
	struct sym560_descriptor *sym560_p = pci_get_drvdata(dev);
	
	/*void cdev_del(struct cdev *dev);*/
	/* unregister the char device */
	cdev_del(&(sym560_p->mycdev));
	
	/* unregister the chrdev region */
	unregister_chrdev_region(MKDEV(SYM560_MAJOR, SYM560_MINOR), 1);

	/* get the descriptor back (created in sys560_probe function) */
	
	printk(KERN_INFO "\nUnregistering the sym560 driver\n\n");
	printk(KERN_DEBUG "Unregistering the following memory:\n");
	printk(KERN_DEBUG "    Virtual Memory Address  = %p\n", sym560_p->vmemaddr);
	printk(KERN_DEBUG "    Base Address Register 2 = %lu\n", sym560_p->memstart);
	printk(KERN_DEBUG "    Length                  = %lu\n", sym560_p->memlen);

	/* unmap the virtual memory */
	iounmap(sym560_p->vmemaddr);
	iounmap(sym560_p->vlcraddr);
	/* release the memory regions */
	release_mem_region(sym560_p->memstart, sym560_p->memlen);
	release_mem_region(sym560_p->lcrstart, sym560_p->lcrlen);
}	
/*****************************************************************************/


/* Create the pci_driver struct which describe the driver to the PCI core */
static struct pci_driver sym560_driver = 
{
	.name = 	"sym560",
	.id_table = 	sym560_ids,
	.probe = 	sym560_probe,
	.remove = 	sym560_remove,
};


/*****************************************************************************/
/* NAME:	init_sym560 
 *
 * ARGUMENTS:	None
 * 
 * RETURNS:	Integer value (0 means succefull???)	
 * 
 * DESCRIPTION: First function that is invoked when insmod is run by the user on
 * 		the command line.  It registers the driver (should now show up 
 * 		in /proc/modules).  The __init means that the kernel scraps this 
 * 		after it has run in order to free up space. 
 */ 		 
static int __init init_sym560(void)
{
	int err;
	err = pci_register_driver(&sym560_driver);
	if (err < 0)
		printk(KERN_ERR "Could not register driver sym560 err = %d\n", err);
	else
		printk(KERN_INFO "sym560 driver was successfully registered\n");
	return err;
}
/*****************************************************************************/


/*****************************************************************************/
/* NAME:	exit_sym560 
 *
 * ARGUMENTS:	None
 * 
 * RETURNS:	Nothing	
 * 
 * DESCRIPTION:	Unregisters the device when rmmod is called.  __exit means it
 * 		is special code that is stored in a special location. 
 */ 		 
static void __exit exit_sym560(void)
{	
	return pci_unregister_driver(&sym560_driver);
}
/*****************************************************************************/


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Krug");

module_init(init_sym560);
module_exit(exit_sym560);
