/* The kernel call implemented in this file:
 *   m_type:	SYS_EXEC
 *
 * The parameters for this kernel call are:
 *    m1_i1:	PR_PROC_NR		(process that did exec call)
 *    m1_p1:	PR_STACK_PTR		(new stack pointer)
 *    m1_p2:	PR_NAME_PTR		(pointer to program name)
 *    m1_p3:	PR_IP_PTR		(new instruction pointer)
 */
/*
 * 该文件实现的系统调用:
 *	m_type:	SYS_EXEC
 *
 * 该系统调用包括的参数包括:
 *	m1_i1:	PR_PROC_NR		调用 exec 的进程号
 *	m1_p1:	PR_STACK_PTR		新的栈指针
 *	m1_p2:	PR_NAME_PTR		程序名
 *	m1_p3:	PR_IP_PTR		新的指令指针(IP)
 */
#include "../system.h"
#include <string.h>
#include <signal.h>

#if USE_EXEC

/*===========================================================================*
 *				do_exec					     *
 *===========================================================================*/
PUBLIC int do_exec(m_ptr)
register message *m_ptr;	/* pointer to request message */
{
/* Handle sys_exec().  A process has done a successful EXEC. Patch it up. */
/* 处理 sys_exec(). 一个进程已经完成了一个成功的 EXEC. 临时接入 */
  register struct proc *rp;
  reg_t sp;			/* new sp */
  phys_bytes phys_name;
  char *np;

  // 获取进程地址
  rp = proc_addr(m_ptr->PR_PROC_NR);
  // 新的栈指针
  sp = (reg_t) m_ptr->PR_STACK_PTR;
  // 为进程设置新的栈指针
  rp->p_reg.sp = sp;		/* set the stack pointer */
  // _phys_memset
  // 将进程的额外的 局部描述表 清空. 
  phys_memset(vir2phys(&rp->p_ldt[EXTRA_LDT_INDEX]), 0,
	(LDT_SIZE - EXTRA_LDT_INDEX) * sizeof(rp->p_ldt[0]));
  // 重置指令指针(IP)
  rp->p_reg.pc = (reg_t) m_ptr->PR_IP_PTR;	/* set pc */
  // 关闭新进程的接收阻塞标志.
  rp->p_rts_flags &= ~RECEIVING;	/* PM does not reply to EXEC call */
  if (rp->p_rts_flags == 0) lock_enqueue(rp);

  /* Save command name for debugging, ps(1) output, etc. */
  // 获取进程名
  phys_name = numap_local(m_ptr->m_source, (vir_bytes) m_ptr->PR_NAME_PTR,
					(vir_bytes) P_NAME_LEN - 1);
  if (phys_name != 0) {
	phys_copy(phys_name, vir2phys(rp->p_name), (phys_bytes) P_NAME_LEN - 1);
	// 找到进程名中第一个字符值小于 空格的字符, 将该字符置了字符串
	// 结束标志.
	for (np = rp->p_name; (*np & BYTE) >= ' '; np++) {}
	*np = 0;					/* mark end */
  } else {
	  // 如果未指定进程名的话, 设置进程名为 "<unset>"
  	strncpy(rp->p_name, "<unset>", P_NAME_LEN);
  }
  return(OK);
}
#endif /* USE_EXEC */
