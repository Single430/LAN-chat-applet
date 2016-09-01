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
		op.sem_num = semnum;// sem_id��Ӧ���ź������еĵ�һ������0��ʾ��0���ź�����
		op.sem_op = -1;// ����ʹ��Կ�ף�������뵽�ˣ�Կ�׸���-1
		op.sem_flg = 0;// ʹ��ϵͳĬ�����ã�
					// IPC_NOWAIT ��ʹû��Կ�ף�Ҳ���ȴ��������������� 
					// SEM_UNDO  ���̽�����ʱ��Կ�����Զ��ָ�֮ǰ��״̬
		
		if(semop(sem_id , &op, 1) == -1)// �˴�1��ʾֻΪһ��������Կ�׵�����
			perror("semop P failed");
}

void sem_v(int sem_id,int semnum)
{
		struct sembuf op;
		op.sem_num = semnum;
		op.sem_op = 1;// �黹Կ�ף�Կ�׸���+1
		op.sem_flg = 0;
		
		if(semop(sem_id , &op, 1) == -1)
			perror("semop V failed");
}

#endif