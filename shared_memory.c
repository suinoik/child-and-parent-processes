#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define BankAccount 0
#define Turn 1

void ChildProcess(int[]);

int main(int argc, char *argv[]) {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int status;

  ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int),
                 IPC_CREAT | 0666); // shared memory for 2 integers
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }
  printf("Process has received a shared memory of 2 integers...\n");

  ShmPTR = (int *)shmat(ShmID, NULL, 0);
  if (*ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }

  ShmPTR[BankAccount] = 0; // Initializing BankAccount
  ShmPTR[Turn] = 0;        // Initializing Turn

  printf("Process has attached the shared memory...\n");
  pid = fork();

  if (pid < 0) {
    printf("fork error\n");
    exit(1);
  } else if (pid == 0) {
    ChildProcess(ShmPTR);
    exit(0);
  } else {
    int j;
    int account;
    int random_deposit;
    printf("Orig Bank Account = %d\n", ShmPTR[BankAccount]);
    srandom(time(NULL));
    for (j = 0; j < 25; j++) {
      // ParentProcess
      sleep(random() % 6);
      while (ShmPTR[Turn] != 0);
      account = ShmPTR[BankAccount];
      if (account <= 100) {
        random_deposit = random() % 101; // for 0-100 inclusive
        if (random_deposit % 2 == 0) {
          account += random_deposit;
          printf("Dear old Dad: Deposits $%d / Balance = $%d\n", random_deposit,
                 account);
        } else {
          printf("Dear old Dad: Doesn't have any money to give\n");
        }
      } else {
        printf("Dear old Dad: Thinks student has enough Cash ($%d)\n", account);
      }
      ShmPTR[BankAccount] = account;
      ShmPTR[Turn] = 1;
    }
    wait(&status);
    printf("Process has detected the completion of its child...\n");
    shmdt((void *)ShmPTR);
    printf("Process has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Process has removed its shared memory...\n");
    printf("Process exits...\n");
    exit(0);
  }
}

void ChildProcess(int SharedMem[]) {
  int i;
  int account;
  int random_withdrawal;
  srandom(time(NULL));
  for (i=0; i<25; i++) {
    sleep(random() % 6);
    while (SharedMem[Turn] != 1);
    account = SharedMem[BankAccount];
    random_withdrawal = random() % 51; // for 0-50 inclusive
    printf("Poor Student needs $%d\n", random_withdrawal);
    if (random_withdrawal <= account) {
      account -= random_withdrawal;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", random_withdrawal,
            account);
    } else {
      printf("Poor Student: Not Enough Cash ($%d)\n", account);
    }
    SharedMem[BankAccount] = account;
    SharedMem[Turn] = 0;
  }
}