// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchcons.h"


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

// Doi thanh ghi Program counter cua he thong ve sau 4 byte de tiep tuc nap lenh
void IncreasePC()
{
	// trong machine/machine.h -- Ham int ReadRegister(int num) doc thanh ghi thu 'num'
	// bien counter doc dia chi PCReg (hien tai)
	int counter = machine->ReadRegister(PCReg); 

	// viet gia tri nay vao thanh ghi truoc do
	// trong machine/machine.h -- Ham void WriteRegister(int num, int value) ghi 'value' vao thanh ghi thu 'num'
	machine->WriteRegister(PrevPCReg, counter); 

	// tiep tuc doc gia tri thanh ghi ke tiep va gan vao thanh ghi hien tai
	counter = machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg, counter);
	
	// viet dia chi cau lenh ke tiep
	machine->WriteRegister(NextPCReg, counter + 4);

	// Thay IncreasePC vao Halt Systemcall
}


char* User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}


// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}




// bien which la loai Exception
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	int op1, op2, result;
	int size;
	char s[200];
	char* s2;
	int a;


	char* buffer;
	int MAX_BUFFER = 255;
	bool isNegative ;
	int firstNumIndex ;
	int lastNumIndex ;
	int numberOfNum;
	int numbytes;
	int number = 0;
	int t_number;
	char c;
	int virtAddr, length;

	switch (which)
	{
		case NoException: 
			return;

		case PageFaultException:
		{
			printf("No valid translation found \n");
			interrupt->Halt();
			break;
		}

		case ReadOnlyException:
		{
			printf("\n\n Write attempted to page marked 'read-only' \n");;
			interrupt->Halt();
			break;
		}
		case BusErrorException:
		{
			printf("\n\n Translation resulted invalid physical address. \n");
			interrupt->Halt();
			break;
		}
		case AddressErrorException:
		{
			printf("\n\n Unaligned reference or one that was beyond the end of the address space. \n");
			interrupt->Halt();
			break;
		}
		case OverflowException:
		{
			printf("\n\n Integer overflow in add or sub. \n");
			interrupt->Halt();
			break;
		}
		case IllegalInstrException:
		{
			printf("\n\n Unimplemented or reserved instr. \n");
			interrupt->Halt();
			break;
		}
		case NumExceptionTypes:
		{
			printf("\n\n Number exception types. \n");
			interrupt->Halt();
			break;
		}
		case SycallException:
		{
			switch (type)
			{
			
			    case SC_Halt:
				{
					DEBUG('a', "Shutdown, initiated by user program. \n);
					interrupt->Halt();
					break;
				}

				case SC_ReadInt:
				{
					// Input: 
					// Output: Tra ve so nguyen doc duoc tu man hinh console.
					// Chuc nang: Doc so nguyen tu man hinh console.

					// Ta su dung bien isNegative de xac dinh so nhap vao la so am hay so duong
					// Ta gia dinh la so duong.
					isNegative = false;

					// firstNumIndex va lastNumIndex se luu vi tri bat dau va ket thuc cua so hop le trong buffer
					firstNumIndex = 0;
					lastNumIndex = 0;

					// cap phat vung nho cho con tro buffer
					buffer = new char[MAX_BUFFER + 1];

					// su dung ham Read(char * buffer, int size ) duoc dinh nghia trong thu vien synchcons
					// ham Read se tra ve so byte cua buffer ma no doc duoc vao bien numbytes
					numbytes = gSynchConsole->Read(buffer, MAX_BUFFER);


					/*------------ Qua trinh chuyen doi tu buffer sang so nguyen int ----------------- */
  
				

					// Dau tien ta se kiem tra chuoi nhap vao la am hay duong thong qua buffer[0]
					if (buffer[0] == '-')
					{
						isNegative = true;
						firstNumIndex = 1;
						lastNumIndex = 1;
					}

					// Kiem tra tinh hop le cua so nguyen duoc nhap vao
					for (int i = firstNumIndex; i < numbytes; i++)
					{
						// Ta se kiem tra xem gia tri tai vi tri i co phai la '.' khong
						// Neu co thi ta se xet tu vi tri sau dau '.' cho den het chuoi
						// Neu cac phan tu trong doan ma ta xet co ton tai mot gia tri khac '0'
						// thi ta se in ra thong bao "Gia tri nhap vao khong phai so nguyen" va tra ve gia tri 0 thong qua ham WriteRegister(2,0)
						// va ket thuc ham
						// Neu trong truong hop cac phan tu trong doan ma ta xet toan la gia tri '0'
						// thi ta se cap nhat lastNumIndex la truoc vi tri chua dau '.'
						if (buffer[i] == '.') /// 324.0000000 van la so
						{
							int j = i + 1;
							for (; j < numbytes; j++)
							{
								if (buffer[j] != '0')
								{
									printf("\n\n Gia tri nhap vao khong phai la so nguyen");
									DEBUG('a', "\n Gia tri nhap vao khong phai la so nguyen");
									machine->WriteRegister(2, 0);
									IncreasePC();
									delete buffer;
									return;
								}
							}
							// la so thoa cap nhat lastNumIndex
							lastNumIndex = i - 1;
							break;
						}

						// Neu truong hop phan tu tai vi tri i khong phai la ki tu so
						// thi ta se thong bao la "Gia tri nhap vao khong phai so nguyen" va tra ve gia tri 0 thong qua ham WriteRegister(2,0)
						// va ket thuc ham
						else if (buffer[i] < '0' && buffer[i] > '9')
						{
							printf("\n\n Gia tri nhap vao khong phai la so nguyen");
							DEBUG('a', "\n Gia tri nhap vao khong phai la so nguyen");
							machine->WriteRegister(2, 0);
							IncreasePC();
							delete buffer;
							return;
						}
						

						// Neu truong hop tat ca cac phan tu i trong buffer khong thoa tat ca cac truong hop tren
						// thi gan vi tri lastNumIndex hop le la tai vi tri i dang xet
						lastNumIndex = i;
					}

					// Sau khi da kiem tra tinh hop le cua so nguyen
					// Ta tien hanh chuyen doi tu chuoi sang so 
					for (int i = firstNumIndex; i <= lastNumIndex; i++)
					{
						number = number * 10 + (int)(buffer[i] - 48);
					}

					// Neu gia tri nhap vao la so am thi ta nhan gia tri chuyen doi truoc do cho -1
					if (isNegative)
					{
						number = number * -1;
					}
					// tra ve gia tri number da duoc chuyen doi thong qua ham WriteRegister(2,number)
					machine->WriteRegister(2, number);
					IncreasePC();

					// Giai phong vung nho buffer sau khi da dung xong
					delete buffer;
					return;
				}


				case SC_PrintInt:
				{
					// Input: mot gia tri kieu Integer
					// Output: 
					// Chuc nang: In so nguyen len man hinh console


					// doc gia tri duoc luu o thanh ghi so 4 vao bien number
					number = machine->ReadRegister(4);

					// Kiem tra xem number nhap vao co phai la so 0 hay khong 
					// Neu la so 0 ta in ra man hinh console thong qua ham Write("0",1) luon ma khong phai xu li gi them
					if (number == 0)
					{
						gSynchConsole->Write("0", 1); // In ra man hinh so 0
						IncreasePC();
						return;
					}


					/*------------Qua trinh chuyen so thanh chuoi de in ra man hinh------------------*/

					// Bien isNegative dung de xac dinh la so am hay duong
					// Ta gia dinh ban dau la so duong
					isNegative = false;

					// Bien numberOfNum dung de luu so chu so cua so nhap vao
					numberOfNum = 0; 

					// Dung de luu vi tri bat dau cua chuoi so
					firstNumIndex = 0;


					// Kiem tra xem so co phai la so am hay khong
					// Neu co ta se chuyen isNegative thanh true
					// Va chuyen so number hien tai thanh so duong bang cach nhan voi -1
					// Va cap nhat lai bien firstNumIndex = 1 vi buffer[0] se dung de chua gia tri dau '-'
					if (number < 0)
					{
						isNegative = true;
						number = number * -1; 
						firstNumIndex = 1;
					}

					// Su dung mot bien tam de gan gia tri number
					t_number = number; 
					// Thuc hien dem so chu so co trong gia tri t_number
					while (t_number)
					{
						numberOfNum++;
						t_number /= 10;
					}


					// Cap phat vung nho cho con tro buffer
					buffer = new char[MAX_BUFFER + 1];

					// Thuc hien chuyen doi so thanh chuoi so va luu vao buffer
					for (int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)
					{
						buffer[i] = (char)((number % 10) + 48);
						number /= 10;
					}

					// Neu la so am thi ta them ki tu '-' vao buffer[0] 
					// Va gan ki tu ket thuc chuoi vao buffer[numberOfNum + 1]
					// Va su dung ham Write(buffer,numberOfNum + 1) de in ra (numberofNum + 1) byte cua chuoi so buffer ra man hinh 
					if (isNegative)
					{
						buffer[0] = '-';
						buffer[numberOfNum + 1] = 0;
						gSynchConsole->Write(buffer, numberOfNum + 1);
						delete buffer;
						IncreasePC();
						return;
					}

					// Neu khong la so am thi
					// ta gan ki tu ket thuc chuoi vao buffer[numberOfNum ]
					// Va su dung ham Write(buffer,numberOfNum ) de in ra (numberofNum) byte cua chuoi so buffer ra man hinh 
					buffer[numberOfNum] = 0;
					gSynchConsole->Write(buffer, numberOfNum);

					// Giai phong vung nho buffer sau khi da su dung xong
					delete buffer;
					IncreasePC();
					return;

				}
			
				case SC_ReadChar:
				{

					//Input: 
					//Output: Tra ve mot ki tu 
					//Cong dung: Doc mot ky tu tu nguoi dung nhap

					// khai bao con tro buffer va cap vung nho cho no
				
					buffer = new char[MAX_BUFFER];

					// gan so byte duoc doc tu bien buffer vao numBytes
					numbytes = gSynchConsole->Read(buffer, MAX_BUFFER);


					// Neu nhap nhieu hon 1 ky tu thi ta se in ra thong bao gia tri nhap vao chi co the la 1 ki tu
					if (numbytes > 1) 
					{
						printf("Gia tri nhap vao chi co the la 1 ki tu!");
						DEBUG('a', "\n Gia tri nhap vao chi co the la 1 ki tu!");
						machine->WriteRegister(2, 0);
						IncreasePC();
						return;
					}

					// Neu khong co ki tu nao duoc nhap vao thi ta se thong bao gia tri nhap vao la rong
					// va tra ve gia tri 0 thong qua ham WriteRegister(2,0)
					else if (numbytes == 0) 
					{
						printf("Gia tri nhap vao la rong!");
						DEBUG('a', "\n Ky tu rong!");
						
						machine->WriteRegister(2, 0);
						IncreasePC();
						return;
					}

					// Neu la ki tu thi ta se tra ve gia tri ki tu c  thong qua ham WriteRegister(2,c)
					else
					{
						
						c = buffer[0];
						machine->WriteRegister(2, c);
					}

					// Giai phong vung nho
					delete buffer;
					break;
				}


				case SC_PrintChar:
				{
					// Input: Ki tu(char)
					// Output: Ki tu(char)
					// Cong dung: Xuat mot ki tu la tham so arg ra man hinh
					// Doc ki tu tu thanh ghi so 4 va gan cho bien c
					c = (char)machine->ReadRegister(4);
					// Ta su dung ham Write(&c,1) de in ra 1 ki tu c ra man hinh
					gSynchConsole->Write(&c, 1);
					//IncreasePC();
					break;

				}

				case SC_ReadString:
				{
					// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
					// Output: 
					// Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da

					 // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
					virtAddr = machine->ReadRegister(4);

					// Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
					length = machine->ReadRegister(5); 

					// Copy chuoi tu vung nho User Space sang System Space
					buffer = User2System(virtAddr, length); 

					// Goi ham Read cua SynchConsole de doc chuoi
					gSynchConsole->Read(buffer, length); 

					// Copy chuoi tu vung nho System Space sang vung nho User Space
					System2User(virtAddr, length, buffer);
					delete buffer;
					IncreasePC(); 
					return;
					//break;
				}

				case SC_PrintString:
				{
					// Input: Buffer(char*)
					// Output: Chuoi doc duoc tu buffer(char*)
					// Cong dung: Xuat mot chuoi la tham so buffer truyen vao ra man hinh
				
					// Lay dia chi cua tham so buffer tu thanh ghi so 4
					virtAddr = machine->ReadRegister(4); 

					// Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai 255 ki tu
					buffer = User2System(virtAddr, 255); 
					length = 0;

					// Dem do dai that cua chuoi
					while (buffer[length] != 0) length++; 

					// Goi ham Write cua SynchConsole de in chuoi
					gSynchConsole->Write(buffer, length + 1);

					// giai phong vung nho buffer 
					delete buffer;
					IncreasePC(); // Tang Program Counter 
					return;
					break;
				}

			}
		}
		default:
			printf("Unexpected user mode exception %d %d \n", which, type);
			break;
	}
    
}
