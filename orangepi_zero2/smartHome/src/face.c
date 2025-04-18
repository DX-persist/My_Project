#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include "face.h"

void face_Init(void)		//将初始化python解释器、导入sys模块、将当前路径添加到sys模块里的path路径全部封装成一个函数
{
	Py_Initialize();	//初始化python解释器，一边调用python的函数、模块
	
	PyObject *sys_Module = PyImport_ImportModule("sys");		//导入sys模块

	PyObject *sys_path = PyObject_GetAttrString(sys_Module,"path");	//获取sys模块里的path属性

	PyList_Append(sys_path,PyUnicode_FromString("."));
	
}

void face_Final(void)
{
	Py_Finalize();
}

double face_identification(void)
{
	double retvalue = 0.0;

	system(WGET_CMD);
	if(access(PICTURE_PATH,F_OK) < 0)
	{
		return retvalue;
	}
	PyObject *pModule = PyImport_ImportModule("face");
	if(!pModule)
	{
		PyErr_Print();
		printf("failed to import pModule\n");
		goto FAILED_MODULE; 
	}

	PyObject *pfunc = PyObject_GetAttrString(pModule,"alibaba_face");
	if(!pfunc)
	{
		PyErr_Print();
		printf("failed to get the attr from pModule\n");
		goto FAILED_PFUNC;
	}
	
	PyObject *callret = PyObject_CallObject(pfunc,NULL);
	if(!callret)
	{
		PyErr_Print();
		printf("failed to call the function\n");
		goto FAILED_CALLRET;
	}


	
	int py_ret = PyArg_Parse(callret,"d",&retvalue);
	if(!py_ret)
	{
		printf("get the return value error\n");
		goto FAILED_RESULT;
	}
	//printf("retvalue = %0.2lf\n",retvalue);

FAILED_RESULT:
	Py_DECREF(callret);
FAILED_CALLRET:
	Py_DECREF(pfunc);
FAILED_PFUNC:
	Py_DECREF(pModule);
FAILED_MODULE:

	return retvalue;
}