#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#define TIMEOUT 1000
#define COOKIE_BUF_SIZE PAGE_SIZE

MODULE_LICENSE("GPL");

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);

int fortune_init(void);
void fortune_exit(void);

struct file_operations fops = 
{
	.read = fortune_read,

	.write = fortune_write,

};

const char *proc_filename = "time-tracker";

char *cookie_buf;

struct proc_dir_entry *proc_file;

static struct timer_list my_timer;

typedef struct window_time {
	char *name;
	int secs;
	int mins;
	int hours;
	int days;
};

struct window_time *programs;
int n, max;

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
	int len = 0;
	int i;
	for (i=0; i < n; i++)
	{
		len += sprintf(buf, "%s: %d:%d:%d:%d\n", programs[i].name, programs[i].days, programs[i].hours, programs[i].mins, programs[i].secs);
	}

	return len;
}

ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
	int free_space = (COOKIE_BUF_SIZE) + 1;

	if (count > free_space)
	{
		printk(KERN_INFO "time-tracker: Cookie pot full.\n");
		return -ENOSPC;
	}

	if (copy_from_user(cookie_buf, buf, count))
	{
        printk( KERN_ERR "time-tracker: ERROR. couldn't copy data from userspace\n" );
		return -EFAULT;
	}

	cookie_buf[count] = 0;

	return count;
}

void inc_time(int i)
{
	programs[i].secs++;
	if (programs[i].secs > 59)
	{
		programs[i].secs = 0;
		programs[i].mins++;
	}
	if (programs[i].mins > 59)
	{
		programs[i].mins = 0;
		programs[i].hours++; 
	}
	if (programs[i].hours > 23)
	{
		programs[i].hours = 0;
		programs[i].days++;
	}
}

void update_stat(char *program_name)
{
	bool checked = false;
	int i;
	for (i = 0; i < n; i++) 
	{
		if (strcmp(programs[i].name, program_name) == 0) 
		{
			inc_time(i);
			checked = true;
		}
	}
	if (!checked)
	{
		if (n == max)
		{
			struct window_time *new_programs;
			new_programs = (struct window_time*)kmalloc(max*2*sizeof(struct window_time), GFP_KERNEL);
			for (i=0; i < n; i++)
			{
				new_programs[i] = programs[i];
			}
			kfree(programs);
			programs = new_programs;
			max = 2*max;
		}
		strcpy(programs[n].name, program_name);
		programs[n].secs = 1;
		programs[n].mins = 0;
		programs[n].hours = 0;
		programs[n].days = 0;
		n++;
	}
}

void my_timer_callback(unsigned long data)
{
	printk( "time-tracker: my_timer_callback called (%ld).\n", jiffies );
  
	printk("time-tracker: %s\n", cookie_buf);
	update_stat(cookie_buf);
  	setup_timer( &my_timer, my_timer_callback, 0 );

  	printk( "time-tracker: Starting timer to fire in (%ld)\n", jiffies );
  	mod_timer( &my_timer, jiffies + msecs_to_jiffies(TIMEOUT) );
}

int fortune_init(void)
{	
	programs = (struct window_time*)kmalloc(10*sizeof(struct window_time), GFP_KERNEL);
	n = 0;
	max = 10;
	if (!programs)
	{
		printk(KERN_INFO "time-tracker: Can't allocate memory for programs array.\n");
		return -ENOMEM;
	}
	// Making virtual /proc/ file
	cookie_buf = vmalloc(COOKIE_BUF_SIZE);
	if (!cookie_buf)
	{
		printk(KERN_INFO "time-tracker: Can't allocate memory for cookie buf.\n");
		return -ENOMEM;
	}

	memset(cookie_buf, 0, COOKIE_BUF_SIZE);

	proc_file = proc_create(proc_filename, 0664, NULL, &fops);

	if (!proc_file)
	{
		kfree(programs);
		vfree(cookie_buf);
		printk(KERN_INFO "time-tracker: Can't create fortune file.\n");
		return -ENOMEM;
	}

	printk("Timer module installing\n");
	
	// my_timer.function, my_timer.data
	setup_timer( &my_timer, my_timer_callback, 0 );
	
	printk( "Starting timer to fire (%ld)\n", jiffies );
	
	if (mod_timer( &my_timer, jiffies + msecs_to_jiffies(TIMEOUT) )) 
		printk("Error in mod_timer\n");
		
	return 0;
}

void fortune_exit(void)
{
	remove_proc_entry(proc_filename, NULL);
	if (cookie_buf)
		vfree(cookie_buf);
	
	if (programs)
		kfree(programs);

	if (del_timer( &my_timer )) 
		printk("The timer is still in use...\n");

	printk("Timer module uninstalling\n");

}

module_init(fortune_init);
module_exit(fortune_exit);