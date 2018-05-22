//Ali Iqbal
//CSC-139
//Assignment 2
//simple.c

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list_sort.h>
static LIST_HEAD(birthday_list);

/* Contains birthday attributes and linked list to hold data */
struct birthday{
	int year;
	int month;
	int day;
	char* name;
	struct list_head list;
};

struct birthday *ptr, *nex;

/* switch case to return name of month from actual month number*/
char* birthMonth(int month){
	switch(month){
		case 1: return "Januray";
		case 2: return "February";
		case 3: return "March";
		case 4: return "April";
		case 5: return "May";
		case 6: return "June";
		case 7: return "July";
		case 8: return "August";
		case 9: return "September";
		case 10: return "October";
		case 11: return "November";
		case 12: return "December";
		default: return "Not a Real Month";
	}
}

/* method compares birthday attributes in linkedList*/
int compareBirthday(void *ptr, struct list_head *one, struct list_head *two){
	struct birthday *brd1 = list_entry(one, struct birthday, list);
	struct birthday *brd2 = list_entry(two, struct birthday, list);

	if(brd1 -> year < brd2 -> year) {
		return -1;
	} else if(brd1 -> year > brd2 -> year){
		return 1;
	} else {
		if(brd1 -> month < brd2 -> month) {
			return -1;
		} else if(brd1 -> month > brd2 -> month){
			return 1;
		} else {
			if(brd1 -> day < brd2 -> day) {
				return -1;
			} else if(brd1 -> day < brd2 -> day){ 
				return 1;
			} else {
				return 0;
			}
		}
	}
}

/* This function is called when the module is loaded. */
int simple_init(void)
{
	struct birthday *alice;
	struct birthday *bob;
	struct birthday *mallory;
	struct birthday *nancy;
	struct birthday *kate;

       printk(KERN_INFO "Loading Module\n");

    alice = kmalloc(sizeof(*alice), GFP_KERNEL);
	alice -> year = 1989;
	alice -> month = 2;
	alice -> day = 15;
	alice -> name = "Alice";
	INIT_LIST_HEAD(&alice -> list);

	bob = kmalloc(sizeof(*bob), GFP_KERNEL);
	bob -> year = 1958;
	bob -> month = 4;
	bob -> day = 8;
	bob -> name = "Bob";
	INIT_LIST_HEAD(&bob -> list);

	mallory = kmalloc(sizeof(*mallory), GFP_KERNEL);
	mallory -> year = 1958;
	mallory -> month = 12;
	mallory -> day = 12;
	mallory -> name = "Mallory";
	INIT_LIST_HEAD(&mallory -> list);

	nancy = kmalloc(sizeof(*nancy), GFP_KERNEL);
	nancy -> year = 2004;
	nancy -> month = 5;
	nancy -> day = 9;
	nancy -> name = "Nancy";
	INIT_LIST_HEAD(&nancy -> list);

	kate = kmalloc(sizeof(*kate), GFP_KERNEL);
	kate -> year = 1988;
	kate -> month = 7;
	kate -> day = 8;
	kate -> name = "Kate";
	INIT_LIST_HEAD(&kate -> list);

	list_add_tail(&alice -> list, &birthday_list);
	list_add_tail(&bob -> list, &birthday_list);
	list_add_tail(&mallory -> list, &birthday_list);
	list_add_tail(&nancy -> list, &birthday_list);
	list_add_tail(&kate -> list, &birthday_list);

	printk("Pre-sorted: \n");
	list_for_each_entry(ptr, &birthday_list,list) {
		printk("%s born on %s %d, %d\n", ptr -> name, 
			birthMonth(ptr -> month), ptr -> day, ptr -> year);
	}
	
	list_sort(NULL, &birthday_list, compareBirthday);

	printk("Post-sorted: \n");
	list_for_each_entry(ptr, &birthday_list,list) {
		printk("%s born on %s %d, %d\n", ptr -> name, 
			birthMonth(ptr -> month), ptr -> day, ptr -> year);
	}

       return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	printk(KERN_INFO "Removing Module\n");
	list_for_each_entry_safe(ptr, nex, &birthday_list, list) {	
		printk("Removing birthday of: %s \n", ptr -> name);		
		/*Remove the list*/
		list_del(&ptr -> list);
		/*free the list memory*/
		kfree(ptr);
	}
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");
