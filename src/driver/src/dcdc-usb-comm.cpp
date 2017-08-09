/* 
 * Copyright (c) 2011 by Mini-Box.com, iTuner Networks Inc.
 * Written by Nicu Pavel <npavel@mini-box.com>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <usb.h>

#include "dcdc-usb.h"

#define P(t, v...) fprintf(stderr, t "\n", ##v)

int dcdc_send(struct usb_dev_handle *h, unsigned char *data, int size)
{
    if (data == NULL)
  return -1;

    return usb_interrupt_write(h, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}

int dcdc_recv(struct usb_dev_handle *h, unsigned char *data, int size, int timeout)
{
    if (data == NULL)
  return -1;

    return usb_interrupt_read(h, USB_ENDPOINT_IN + 1, (char *) data, size, timeout);
}

struct usb_dev_handle * dcdc_connect()
{
    struct usb_bus *b;
    struct usb_device *d;
    struct usb_dev_handle *h = NULL;
    
    usb_init();
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();

    for (b = usb_get_busses(); b != NULL; b = b->next)
    {
      for (d = b->devices; d != NULL; d = d->next)
      {
          if ((d->descriptor.idVendor == DCDC_VID) && 
             (d->descriptor.idProduct == DCDC_PID))
          {
            h = usb_open(d);
            char name[1024];
            usb_get_string_simple(h, 2, name, sizeof(name)-1);

            if (h)
            {
                char buffer[256];
                usb_reset(h);
                printf("usb_open ( %s ) sucessfull %p , vendor:%x \n", name,d,d->descriptor.idVendor);

                usb_get_string_simple(h, d->descriptor.iManufacturer, buffer, sizeof(buffer));
                printf("- Manufacturer: %s\n", buffer);

                usb_get_string_simple(h, d->descriptor.iProduct, buffer, sizeof(buffer));
                printf("- Product: %s\n", buffer);

                usb_get_string_simple(h, d->descriptor.iSerialNumber, buffer, sizeof(buffer));
                printf("- Serialnumber: %s\n", buffer);

                printf("- Number of configurations: %d\n", d->descriptor.bNumConfigurations );               
                
            }
            else printf("usb_open ( %s ) failed %p \n", name,d);
            
            break;
          }
      }
    }
  return h;
}


int dcdc_setup(struct usb_dev_handle *h)
{
    char buf[65535];
    
    if (h == NULL)
      return -1;
  
    if (usb_get_driver_np(h, 0, buf, sizeof(buf)) == 0)
    {
        if (usb_detach_kernel_driver_np(h, 0) < 0)
        {
          fprintf(stderr, "Cannot detach from kernel driver\n");
          return -2;
        }
    }
    
    if (usb_set_configuration(h, 1) < 0)
    {
        fprintf(stderr, "Cannot set configuration 1 for the device\n");
        return -3;
    }
    
    usleep(1000);
    
    if (usb_claim_interface(h, 0) < 0)
    {
        fprintf(stderr, "Cannot claim interface 0\n");
        return -4;
    }

    if (usb_set_altinterface(h, 0) < 0)
    {
        fprintf(stderr, "Cannot set alternate configuration\n");
        return -5;
    }
    
    if (usb_control_msg(h, USB_TYPE_CLASS + USB_RECIP_INTERFACE, 
        0x000000a, 0x0000000, 0x0000000, buf, 0x0000000, 1000) 
        < 0)
    {
        fprintf(stderr, "Cannot send control message\n");
        return -6;
    }

    return 0;
}
