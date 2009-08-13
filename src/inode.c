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

static struct inode_operations cfs_dir_inode_operations;
static struct inode_operations cfs_file_inode_operations;
static struct file_operations cfs_file_operations;
static struct address_space_operations cfs_aops;
struct kmem_cache *cfs_inode_cachep;

/*
 * super ops
 */
struct inode *cfs_alloc_inode (struct super_block *sb)
{
	struct cfs_inode *ei;
	struct inode *inode;

	ei = (struct cfs_inode *) kmem_cache_alloc (cfs_inode_cachep, GFP_KERNEL);
	if (!ei)
		return NULL;
	inode = &ei->vfs_inode;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;

	return inode;
}

void cfs_destroy_inode (struct inode *inode)
{
	kmem_cache_free (cfs_inode_cachep, CFS_I(inode));
}

static void init_once (void *data)
{
	struct cfs_inode *ei = (struct cfs_inode *) data;

	inode_init_once (&ei->vfs_inode);
}

int cfs_init_cachep (void) {
	cfs_inode_cachep = kmem_cache_create ("cfs_inode_cachep",
		sizeof (struct cfs_inode), 0, 0, init_once);

	if (!cfs_inode_cachep) {
		cfs_destroy_cachep ();
		return -ENOMEM;
	}
	return 0;
}

void cfs_destroy_cachep (void) {
	if (cfs_inode_cachep)
		kmem_cache_destroy (cfs_inode_cachep);
}

/*
 * inode ops
 */
struct inode *cfs_make_inode (struct super_block *sb, int mode, dev_t dev)
{
	struct inode *inode;
	struct cfs_inode *ei;

	inode = new_inode (sb);
	if (!inode)
		goto out;
	ei = CFS_I (inode);

	printk (KERN_INFO "cfs_make_inode\n");

	inode->i_mode = mode;
	inode->i_uid = inode->i_gid = 0;
	inode->i_blocks = 0;
	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	switch (mode & S_IFMT) {
		case S_IFREG:
			inode->i_op = &cfs_file_inode_operations;
			inode->i_fop = &cfs_file_operations;
//			inode->i_mapping->a_ops = &cfs_aops;
			break;
		case S_IFDIR:
			inode->i_op = &cfs_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;
			break;
		default:
			init_special_inode (inode, mode, dev);
			break;
	}
out:
	return inode;
}

static int cfs_mknod (struct inode *dir, struct dentry *dentry, int mode, dev_t dev)
{
	struct inode *inode = cfs_make_inode (dir->i_sb, mode, dev);
	int error = -ENOSPC;

	if (inode) {
		d_instantiate (dentry, inode);
		dget (dentry);
		error = 0;
		dir->i_mtime = dir->i_ctime = CURRENT_TIME;
	}

	return error;
}

static int cfs_mkdir (struct inode *dir, struct dentry *dentry, int mode)
{
	int retval = cfs_mknod (dir, dentry, mode | S_IFDIR, 0);

	return retval;
}

static int cfs_create (struct inode *dir, struct dentry *dentry, int mode, struct nameidata *nd)
{
	return cfs_mknod (dir, dentry, mode | S_IFREG, 0);
}

static int cfs_symlink (struct inode *dir, struct dentry *dentry, const char *symname)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = cfs_make_inode (dir->i_sb, S_IFLNK|S_IRWXUGO, 0);
	if (inode) {
		error = page_symlink (inode, symname, strlen (symname) + 1);
		if (!error) {
			d_instantiate (dentry, inode);
			dget (dentry);
			dir->i_mtime = dir->i_ctime = CURRENT_TIME;
		} else
			iput (inode);
	}
	return error;
}

/*
 * address_space ops
 */
int cfs_readpage (struct file *file, struct page *page)
{
	printk (KERN_INFO "cfs_readpage\n");
	return 0;
}

int cfs_writepage (struct page *page, struct writeback_control *wbc)
{
	printk (KERN_INFO "cfs_writepage\n");
	return 0;
}

/*
 * file system struct definitions
 */
static struct inode_operations cfs_dir_inode_operations = {
	.create		= cfs_create,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
	.symlink	= cfs_symlink,
	.mkdir		= cfs_mkdir,
	.rmdir		= simple_rmdir,
	.mknod		= cfs_mknod,
	.rename		= simple_rename,
};

static struct address_space_operations cfs_aops = {
	.readpage	= cfs_readpage,
	.writepage	= cfs_writepage,
};
