import ctypes
import numpy as np

# 加载共享库
lib = ctypes.cdll.LoadLibrary('./clib/libtest_sort.so')

# 定义 C++ 函数参数和返回值类型
lib.sort_array.argtypes = [np.ctypeslib.ndpointer(dtype=np.double, ndim=1, flags='C_CONTIGUOUS'), ctypes.c_int]
lib.sort_array.restype = None

# 创建一个示例数组
arr = np.array([3.2, 1.5, 4.7, 2.1, 5.0])

# 调用 C++ 函数对数组进行排序
for i in range(5):
    lib.sort_array(arr, len(arr))

    # 打印排序后的数组
    print(arr)
