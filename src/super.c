/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/smp_lock.h>
#include <linux/vfs.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
#include <linux/log2.h>
#include <linux/quotaops.h>
#include <asm/uaccess.h>
#include "cfs.h"

#define CFS_SUPER_MAGIC 0x19980122

static void cfs_put_super (struct super_block *sb)
{
	struct cfs_sb_info *sbi = sb->s_fs_info;

	printk (KERN_INFO "cfs_put_super\n");

	sb->s_fs_info = NULL;
	kfree (sbi);
}

static void cfs_write_super (struct super_block *sb)
{
	sb->s_dirt = 0;
}

static int cfs_statfs (struct dentry *dentry, struct kstatfs *buf)
{
	struct super_block *sb = dentry->d_sb;
	struct cfs_sb_info *sbi = sb->s_fs_info;

	buf->f_type = CFS_SUPER_MAGIC;
	buf->f_bsize = sb->s_blocksize;
	buf->f_blocks = 1024;
	buf->f_bfree = 1023;
	buf->f_bavail = 1023;

	return 0;
}

static const struct super_operations cfs_s_ops = {
	.alloc_inode	= cfs_alloc_inode,
	.destroy_inode	= cfs_destroy_inode,
	.put_super	= cfs_put_super,
	.write_super	= cfs_write_super,
	.statfs		= cfs_statfs,
	.show_options	= generic_show_options,
};

static int cfs_fill_super (struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	struct dentry *root_dentry;
	struct cfs_sb_info *sbi;

	printk (KERN_INFO "cfs_fill_super\n");
	printk (KERN_INFO "MAGIC %lx\n", sb->s_magic);

	sbi = kzalloc (sizeof (struct cfs_sb_info), GFP_KERNEL);
	if (!sbi)
		return -ENOMEM;
	sb->s_fs_info = sbi;
	/* defining block size, magic number and superblock ops */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	sb->s_magic = CFS_SUPER_MAGIC;
	sb->s_op = &cfs_s_ops;

	/* creating the root inode */
	root = cfs_make_inode (sb, S_IFDIR | 0755, 0);
	if (! root)
		goto out;

	/* create the root directory */
	root_dentry = d_alloc_root (root);
	if (! root_dentry)
		goto out;
	sb->s_root = root_dentry;

	//cfs_create_files (sb, root_dentry);
	return 0;

out:
	return -ENOMEM;
}

static int cfs_get_sb (struct file_system_type *fs_type, int flags,
	const char *dev_name, void *data, struct vfsmount *mnt)
{
	printk (KERN_INFO "cfs_get_sb %s\n", dev_name);

	return get_sb_bdev (fs_type, flags, dev_name, data, cfs_fill_super, mnt);
}

static struct file_system_type cfs_type = {
	.owner		= THIS_MODULE,
	.name		= "cfs",
	.get_sb		= cfs_get_sb,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int __init init_cfs (void)
{
	int err;
	printk (KERN_INFO "init_cfs\n");

	err = cfs_init_cachep ();
	if (err) return err;

	return register_filesystem (&cfs_type);
}

static void __exit exit_cfs (void)
{
	printk (KERN_INFO "exit_cfs\n");

	cfs_destroy_cachep ();
	unregister_filesystem (&cfs_type);
}

MODULE_AUTHOR ("Wilson Felipe");
MODULE_DESCRIPTION ("A compressed filesystem");
MODULE_LICENSE ("GPL");
module_init (init_cfs);
module_exit (exit_cfs);
