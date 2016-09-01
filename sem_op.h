#ifndef __SEM_OP_H__
#define __SEM_OP_H__

union semun{
				int val;
				struct semid_ds *buf;
				unsigned short *array;
				struct seminfo *_buf;
};

void sem_p(int sem_id,int semnum)
{
		struct sembuf op;
		op.sem_num = semnum;// sem_id对应的信号量组中的第一把锁（0表示第0个信号量）
		op.sem_op = -1;// 申请使用钥匙，如果申请到了，钥匙个数-1
		op.sem_flg = 0;// 使用系统默认设置，
					// IPC_NOWAIT 即使没有钥匙，也不等待，继续往下运行 
					// SEM_UNDO  进程结束的时候钥匙数自动恢复之前的状态
		
		if(semop(sem_id , &op, 1) == -1)// 此处1表示只为一个锁设置钥匙的数量
			perror("semop P failed");
}

void sem_v(int sem_id,int semnum)
{
		struct sembuf op;
		op.sem_num = semnum;
		op.sem_op = 1;// 归还钥匙，钥匙个数+1
		op.sem_flg = 0;
		
		if(semop(sem_id , &op, 1) == -1)
			perror("semop V failed");
}

#endif