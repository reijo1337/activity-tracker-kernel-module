#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/timer.h>

#define TIMEOUT 10000

MODULE_LICENSE("GPL");

int fortune_init(void);
void fortune_exit(void);

static struct timer_list my_timer;

void my_timer_callback( unsigned long data )
{
	printk( "my_timer_callback called (%ld).\n", jiffies );
  
  	setup_timer( &my_timer, my_timer_callback, 0 );

  	printk( "Starting timer to fire in (%ld)\n", jiffies );
  	mod_timer( &my_timer, jiffies + msecs_to_jiffies(TIMEOUT) );
}

int fortune_init(void)
{
	int ret;
	
	printk("Timer module installing\n");
	
	// my_timer.function, my_timer.data
	setup_timer( &my_timer, my_timer_callback, 0 );
	
	printk( "Starting timer to fire (%ld)\n", jiffies );
	ret = mod_timer( &my_timer, jiffies + msecs_to_jiffies(TIMEOUT) );
	
	if (ret) 
		printk("Error in mod_timer\n");
		
	return 0;
}

void fortune_exit(void)
{
	int ret;
	
	ret = del_timer( &my_timer );
	
	if (ret) 
		printk("The timer is still in use...\n");

	printk("Timer module uninstalling\n");
}

module_init(fortune_init);
module_exit(fortune_exit);