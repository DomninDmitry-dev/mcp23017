#include <linux/kernel.h>
#include <linux/module.h>
//#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
//#include <linux/delay.h>
//#include <linux/interrupt.h>
//#include <asm/irq.h>
//#include <linux/init.h>
#include <linux/kernel.h>
//#include <linux/platform_device.h>
//#include <linux/gpio/consumer.h>
//#include <linux/of.h>
//#include <linux/spinlock.h>

/*----------------------------------------------------------------------------*/
#define GPIO_NUM 16
#define INPUT 1
#define OUTPUT 0

/* Chip Control registers */

/*
 * GP0 ang GP1 provides access to GPIO ports
 * A read from these register provide status of pin of these ports
 * A write to these register will modify the output latch register (OLAT0, OLAT1)
 * and data register.
 */
#define GP0		0x12
#define GP1		0x13

/*
 * IOLAT0 and IOLAT1 control the output value of GP0 and GP1
 * Write into one of these register will result in affecting
 */
// Фиксируются биты при записи в порт
#define OLAT0	0x14
#define OLAT1	0x15

// Регистры выбора полярности
#define IPOL0	0x02
#define IPOL1	0x03

/*
 * IODIR0 and IODIR1 registers control GP0 and GP1 IOs direction
 * 1 => input; 0 => output
 * default value are 0xFF in each reg.
 */
#define IODIR0	0x00
#define IODIR1	0x01

/*
 * INTCAP0 and INTCAP1 register contain value of the port that generated the interupt
 * INTCAP0 contains value of GPO at time of GPO change interrupt
 * INTCAP1 contains value of GP1 at time of GP1 change interrupt
 */
#define INTCAP0	0x10
#define INTCAP1	0x11

#define IOCON0 0x0A
#define IOCON1 0x0B

#define MS_TO_NS(x) (x * 1E6L)

/*----------------------------------------------------------------------------*/
struct mcp23017 {
	struct i2c_client *client;
	struct gpio_chip chip;
	u16 irq_enable;
	u16 iodir;
	u16 ioport;
	unsigned int edge[GPIO_NUM];
	struct mutex lock;
	struct mutex irq_lock;
};

//static char irqName[20] = "I2C: ";

/*----------------------------------------------------------------------------*/
static inline struct mcp23017 *to_mcp23017(struct gpio_chip *gc)
{
	return container_of(gc, struct mcp23017, chip);
}
/*----------------------------------------------------------------------------*/
// Вызывается когда разрешается прерывание
// void mcp23017_gpio_irq_unmask(struct irq_data *data)
// {
// 	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
// 	struct mcp23017 *mcp = to_mcp23017(gc);

// 	dev_info(&mcp->client->dev, "mcp23017_gpio_irq_unmask\n");

// 	dev_info(&mcp->client->dev, "hwirq - %ld\n", data->hwirq);
// 	dev_info(&mcp->client->dev, "irq_base - %d\n", mcp->chip.base);
// 	dev_info(&mcp->client->dev, "irq - %d\n", data->irq);

// 	mcp->irq_enable |= BIT(data->hwirq);
// 	dev_info(&mcp->client->dev, "irq_enable = 0x%x\n", mcp->irq_enable);
// }
/*----------------------------------------------------------------------------*/
// Вызывается когда запрещается прерывание
// void mcp23017_gpio_irq_mask(struct irq_data *data)
// {
// 	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
// 	struct mcp23017 *mcp = to_mcp23017(gc);

// 	dev_info(&mcp->client->dev, "mcp23017_gpio_irq_mask\n");

// 	mcp->irq_enable &= ~BIT(data->hwirq);
// 	mcp->edge[data->hwirq] = 0;

// 	dev_info(&mcp->client->dev, "irq_enable = 0x%x\n", mcp->irq_enable);
// }
/*----------------------------------------------------------------------------*/
// Вызывается когда вводится тип прерывания:
// rising - 1
// falling - 2
// both - 3
// int	mcp23017_gpio_irq_set_type(struct irq_data *data, unsigned int flow_type)
// {
// 	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
// 	struct mcp23017 *mcp = to_mcp23017(gc);

// 	dev_info(&mcp->client->dev, "mcp23017_gpio_irq_set_type, flow_type - 0x%x\n", flow_type);
// 	mcp->edge[data->hwirq] = flow_type;
// 	return 0; // Присвоение типа прерывания удачно
// }
/*----------------------------------------------------------------------------*/
// static void mcp23017_irq_bus_lock(struct irq_data *data)
// {
// 	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
// 	struct mcp23017 *mcp = to_mcp23017(gc);

// 	dev_info(&mcp->client->dev, "mcp23017_irq_bus_lock\n");
// 	mutex_lock(&mcp->irq_lock);
// }
/*----------------------------------------------------------------------------*/
// static void mcp23017_irq_bus_unlock(struct irq_data *data)
// {
// 	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
// 	struct mcp23017 *mcp = to_mcp23017(gc);

// 	dev_info(&mcp->client->dev, "mcp23017_irq_bus_unlock\n");
// 	mutex_unlock(&mcp->irq_lock);
// }
/*----------------------------------------------------------------------------*/
// static struct irq_chip mcp23017_irq_chip = {
// 	.name = "mcp23017-gpio",
// 	.irq_mask = mcp23017_gpio_irq_mask,
// 	.irq_unmask = mcp23017_gpio_irq_unmask,
// 	.irq_set_type = mcp23017_gpio_irq_set_type,
// 	.irq_bus_lock = mcp23017_irq_bus_lock,
// 	.irq_bus_sync_unlock = mcp23017_irq_bus_unlock,
// };
/*----------------------------------------------------------------------------*/
// static irqreturn_t mcp23017_irq(int irq, void *data)
// {
// 	struct mcp23017 *mcp = data;
// 	unsigned int child_irq;
// 	unsigned long gpio;
// 	s32 iodirval;
// 	u16 irqMask;

// 	dev_info(&mcp->client->dev, "***************************************\n");

// 	mutex_lock(&mcp->lock);
// 	iodirval = i2c_smbus_read_byte_data(mcp->client, INTCAP0);
// 	irqMask = (u16)iodirval;
// 	dev_info(&mcp->client->dev, "INTCAP0: 0x%x\n", iodirval);
// 	iodirval = i2c_smbus_read_byte_data(mcp->client, INTCAP1);
// 	irqMask |= (u16)iodirval << 8;
// 	dev_info(&mcp->client->dev, "INTCAP1: 0x%x\n", iodirval);
// 	dev_info(&mcp->client->dev, "irqMask: 0x%x\n", irqMask);
// 	dev_info(&mcp->client->dev, "irq_enable: 0x%x\n", mcp->irq_enable);
// 	mutex_unlock(&mcp->lock);

// 	// Перебираем активные прерывания
// 	for(gpio=0; gpio<mcp->chip.ngpio; gpio++)
// 	{
// 		if(mcp->irq_enable & BIT(gpio))
// 		{
// 			//  Проверяем тип прерывания: rising, falling, both
// 			if( (mcp->edge[gpio] == IRQ_TYPE_EDGE_BOTH) ||
// 				( (mcp->edge[gpio] == IRQ_TYPE_EDGE_RISING) && (irqMask & BIT(gpio)) ) ||
// 				( (mcp->edge[gpio] == IRQ_TYPE_EDGE_FALLING) && (!(irqMask & BIT(gpio))) )
// 			) {
// 				child_irq = irq_find_mapping(mcp->chip.irq.domain, gpio);
// 				dev_info(&mcp->client->dev, "irq_find_mapping: gpio = %ld, child_irq = %d\n", gpio, child_irq);
// 				handle_nested_irq(child_irq);
// 			}
// 		}
// 	}

// 	return (irqreturn_t)IRQ_HANDLED;
// }
/*----------------------------------------------------------------------------*/
static int mcp23017_get_value(struct gpio_chip *gc, unsigned offset)
{
	s32 value;
	struct mcp23017 *mcp = to_mcp23017(gc);
	unsigned bank = offset / 8 ;
	unsigned bit = offset % 8 ;

	u8 reg_intcap = (bank == 0) ? INTCAP0 : INTCAP1;
	value = i2c_smbus_read_byte_data(mcp->client, reg_intcap);

	return (value >= 0) ? (value >> bit) & 0x1 : 0;
}
/*----------------------------------------------------------------------------*/
static int mcp23017_set(struct mcp23017 *mcp, unsigned offset, int val)
{
	s32 value;

	unsigned bank = offset / 8 ;
	u8 reg_gpio = (bank == 0) ? GP0 : GP1;
	unsigned bit = offset % 8 ;

	value = i2c_smbus_read_byte_data(mcp->client, reg_gpio);
	if (value >= 0) {
		if (val)
			value |= 1 << bit;
		else
			value &= ~(1 << bit);

		mcp->ioport = value;

		return i2c_smbus_write_byte_data(mcp->client, reg_gpio, value);
	}

	return value;
}
/*----------------------------------------------------------------------------*/
static void mcp23017_set_value(struct gpio_chip *gc, unsigned offset, int val)
{
	struct mcp23017 *mcp = to_mcp23017(gc);

	mutex_lock(&mcp->lock);
	mcp23017_set(mcp, offset, val);
	mutex_unlock(&mcp->lock);
}
/*----------------------------------------------------------------------------*/
/*
 * direction = 1 => input
 * direction = 0 => output
 */
static int mcp23017_direction(struct gpio_chip *gc, unsigned offset,
                                unsigned direction, int val)
{
	struct mcp23017 *mcp = to_mcp23017(gc);
	// Select port A or B
	// offset = 0 to 15
	unsigned bank = offset / 8 ;
	unsigned bit = offset % 8 ;
	u8 reg_iodir = (bank == 0) ? IODIR0 : IODIR1;
	s32 iodirval = i2c_smbus_read_byte_data(mcp->client, reg_iodir);

	if (direction)
		iodirval |= 1 << bit;
	else
		iodirval &= ~(1 << bit);

	mcp->iodir = iodirval;

	i2c_smbus_write_byte_data(mcp->client, reg_iodir, iodirval);

	if (direction)
		return iodirval ;
	else
		return mcp23017_set(mcp, offset, val);
}
/*----------------------------------------------------------------------------*/
static int mcp23017_direction_output(struct gpio_chip *gc,
                                    unsigned offset, int val)
{
	int res;
	struct mcp23017 *mcp = to_mcp23017(gc);

	dev_info(&mcp->client->dev, "Direction output\n");
	mutex_lock(&mcp->lock);
	res = mcp23017_direction(gc, offset, OUTPUT, val);
	mutex_unlock(&mcp->lock);
	return res;
}
/*----------------------------------------------------------------------------*/
static int mcp23017_direction_input(struct gpio_chip *gc,
                                    unsigned offset)
{
	int res;
	struct mcp23017 *mcp = to_mcp23017(gc);

	dev_info(&mcp->client->dev, "Direction input\n");
	mutex_lock(&mcp->lock);
	res = mcp23017_direction(gc, offset, INPUT, 0);
	mutex_unlock(&mcp->lock);
	return res;
}
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_DEBUG_FS
#include <linux/seq_file.h>
static void mcp23017_dbg_show(struct seq_file *s, struct gpio_chip *gc)
{
	struct mcp23017 *mcp = to_mcp23017(gc);
	int		t;
	unsigned	mask;

	for (t = 0, mask = 1; t < mcp->chip.ngpio; t++, mask <<= 1) {
		const char	*label;

		label = gpiochip_is_requested(&mcp->chip, t);
		if (!label)
			continue;

		seq_printf(s, " gpio-%-3d (%d) (%-12s) %s %s",
			mcp->chip.base + t, t, label,
			(mcp->iodir & mask) ? "in " : "out",
			(mcp->ioport & mask) ? "hi" : "lo");
		/* NOTE:  ignoring the irq-related registers */
		seq_puts(s, "\n");
	}
}
#else
#define mcp23017_dbg_show	NULL
#endif
/*----------------------------------------------------------------------------*/
static int mcp23017_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	struct mcp23017 *mcp;
	int retval;

	dev_info(&client->dev, "Starting module\n");

	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "Error i2c <i2c_check_functionality> \n");
		return -EIO;
	}

	mcp = devm_kzalloc(&client->dev, sizeof(*mcp), GFP_KERNEL);
	if (!mcp) {
		dev_err(&client->dev, "Error mem <devm_kzalloc> \n");
		return -ENOMEM;
	}

	mutex_init(&mcp->lock);

	mcp->chip.label = client->name;
	mcp->chip.base = -1;
	mcp->chip.parent = &client->dev;
	mcp->chip.owner = THIS_MODULE;
	mcp->chip.ngpio = GPIO_NUM;
	mcp->chip.can_sleep = true;
	mcp->chip.get = mcp23017_get_value;
	mcp->chip.set = mcp23017_set_value;
	mcp->chip.direction_output = mcp23017_direction_output;
	mcp->chip.direction_input = mcp23017_direction_input;
	if (IS_ENABLED(CONFIG_DEBUG_FS)) {
		mcp->chip.dbg_show = mcp23017_dbg_show;
		dev_info(&client->dev, "Activate mcp23017_dbg_show \n");
	}

	mcp->client = client;

	//mutex_init(&mcp->irq_lock);
	//client->irq = platform_get_irq(pdev, 0);

	/* Do we have an interrupt line? Enable the irqchip */
	// if (client->irq) {
		//strcat(irqName, dev_name(&client->dev));

		//dev_info(&mcp->client->dev, "fucn - devm_request_threaded_irq \n");
		//retval = devm_request_threaded_irq(&client->dev, client->irq, NULL,
		//										mcp23017_irq,
		//										IRQF_ONESHOT |
		//										IRQF_TRIGGER_FALLING |
		//										IRQF_SHARED,
		//										irqName, mcp);
		//if (retval) {
		//	dev_err(&mcp->client->dev, "Error irq <devm_request_threaded_irq>, unable to request IRQ#%d: %d\n",
		//			client->irq, retval);
		//	goto fail;
		//}

		// dev_info(&mcp->client->dev, "fucn - gpiochip_irqchip_add \n");
		// gpiochip_irqchip_add
		// gpiochip_irqchip_add_nested
		// retval = gpiochip_irqchip_add_nested(&mcp->chip,
											// &mcp23017_irq_chip,
											// 0, handle_level_irq, IRQ_TYPE_NONE);
		// if (retval) {
			// dev_err(&mcp->client->dev, "Cannot add irqchip \n");
			// goto fail;
		// }

		// dev_info(&mcp->client->dev, "fucn - gpiochip_set_chained_irqchip \n");
		//gpiochip_set_chained_irqchip
		//gpiochip_set_nested_irqchip
		// gpiochip_set_nested_irqchip(&mcp->chip,
									// &mcp23017_irq_chip,
									// client->irq);
		// dev_info(&mcp->client->dev, "irq - %d\n", client->irq);
	// }

	retval = gpiochip_add(&mcp->chip);
	if(retval)
		goto fail;

	i2c_set_clientdata(client, mcp);

	return 0;

fail:
	dev_err(&client->dev, "Can't setup chip \n");
	return -1;
}
/*----------------------------------------------------------------------------*/
static int mcp23017_remove(struct i2c_client *client)
{
	struct mcp23017 *mcp = i2c_get_clientdata(client);

	mutex_lock(&mcp->lock);
	i2c_smbus_write_byte_data(client, IODIR0, 0x00);
	i2c_smbus_write_byte_data(client, IODIR1, 0x00);
	i2c_smbus_write_byte_data(client, GP0, 0x00);
	i2c_smbus_write_byte_data(client, GP1, 0x00);
	mutex_unlock(&mcp->lock);

	gpiochip_remove(&mcp->chip);
	//synchronize_irq(client->irq);

	dev_info(&client->dev, "Removing module \n");
	return 0;
}
/*----------------------------------------------------------------------------*/
static const struct of_device_id mcp23017_ids[] = {
	{ .compatible = "microchip,mcp23017-i2c", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, mcp23017_ids);
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id mcp23017_id[] = {
	{"mcp23017", 0},
	{},
};
/*----------------------------------------------------------------------------*/
MODULE_DEVICE_TABLE(i2c, mcp23017_id);

static struct i2c_driver mcp23017_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "mcp23017",
		.of_match_table = of_match_ptr(mcp23017_ids),
	},
	.probe = mcp23017_probe,
	.remove = mcp23017_remove,
	.id_table = mcp23017_id,
};

module_i2c_driver(mcp23017_i2c_driver);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Dmitry Domnin");
MODULE_DESCRIPTION("MCP23017 I2C expander module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
