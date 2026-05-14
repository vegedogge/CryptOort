# import FL_runner
# import time
# import numpy as np

# import json
# import random
# from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
# from cryptography.hazmat.backends import default_backend
# from cryptography.hazmat.primitives import padding
# from cryptography.hazmat.primitives.asymmetric import rsa
# from cryptography.hazmat.primitives import serialization
# from cryptography.hazmat.primitives.asymmetric import padding as asymmetric_padding
# from cryptography.hazmat.primitives import hashes


# def add_padding(data):
#     padder = padding.PKCS7(128).padder()
#     padded_data = padder.update(data) + padder.finalize()
#     return padded_data

# def remove_padding(data):
#     unpadder = padding.PKCS7(128).unpadder()
#     unpadded_data = unpadder.update(data) + unpadder.finalize()
#     return unpadded_data

# def encrypt_data(key, data):
#     backend = default_backend()
#     cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=backend)
#     encryptor = cipher.encryptor()
#     ct = encryptor.update(data) + encryptor.finalize()
#     return ct

# def decrypt_data(key, ct):
#     backend = default_backend()
#     cipher = Cipher(algorithms.AES(key), modes.ECB(), backend=backend)
#     decryptor = cipher.decryptor()
#     pt = decryptor.update(ct) + decryptor.finalize()
#     return pt

# def generate_rsa_key():
#     private_key = rsa.generate_private_key(
#         public_exponent=65537,
#         key_size=2048,
#         backend=default_backend()
#     )
#     public_key = private_key.public_key()
#     return private_key, public_key

# # 定义函数：使用公钥加密 AES 密钥
# def encrypt_aes_key(aes_key, public_key):
#     encrypted_key = public_key.encrypt(
#         aes_key,
#         asymmetric_padding.OAEP(
#             mgf=asymmetric_padding.MGF1(algorithm=hashes.SHA256()),
#             algorithm=hashes.SHA256(),
#             label=None
#         )
#     )
#     return encrypted_key

# # 定义函数：使用私钥解密 AES 密钥
# def decrypt_aes_key(encrypted_key, private_key):
#     aes_key = private_key.decrypt(
#         encrypted_key,
#         asymmetric_padding.OAEP(
#             mgf=asymmetric_padding.MGF1(algorithm=hashes.SHA256()),
#             algorithm=hashes.SHA256(),
#             label=None
#         )
#     )
#     return aes_key

# # 生成 RSA 密钥对
# private_key, public_key = generate_rsa_key()

# # 获取模型参数
# global_model=FL_runner.CNN_model_factory()
# global_weight = global_model.get_weights()

# # 将 Numpy 数组转换为 Python 基本数据类型
# global_weight_list = [arr.tolist() for arr in global_weight]

# s=time.time()
# # 转换为 JSON 字符串
# data_to_encrypt = json.dumps(global_weight_list).encode('utf-8')

# # 添加填充
# padded_data = add_padding(data_to_encrypt)

# # AES 密钥（需要是 16、24 或 32 字节）
# key = b'16byteSecretKey!'

# # 使用 AES 加密数据
# # 使用公钥加密 AES 密钥
# encrypted_aes_key = encrypt_aes_key(key, public_key)
# encrypted_data = encrypt_data(key, padded_data)
# e=time.time()
# print("En: ", e-s)

# # 使用 AES 解密数据
# s=time.time()
# # 使用私钥解密 AES 密钥
# decrypted_aes_key = decrypt_aes_key(encrypted_aes_key, private_key)
# decrypted_data = decrypt_data(key, encrypted_data)

# # 去除填充
# unpadded_data = remove_padding(decrypted_data)

# # 解析 JSON 数据并将其转换回 Numpy 数组
# decrypted_global_weight_list = json.loads(unpadded_data.decode('utf-8'))
# restored_global_weight = [np.array(arr) for arr in decrypted_global_weight_list]

# e=time.time()
# print("De: ", e-s)

# hash承诺
import time
import random
import FL_runner
import hashlib

global_model=FL_runner.CNN_model_factory()
global_weight = global_model.get_weights()
flattened_params = [item for w in global_weight for item in w.flatten()]
bytes_str = bytes(str(flattened_params), 'utf-8')

# 生成一个随机数
random_num = random.randint(1, 100)
integer_num = 123456789

start_time = time.time()

# 对随机数和整数进行哈希
hashed_value = hashlib.sha256(bytes_str).hexdigest()

end_time = time.time()
execution_time = end_time - start_time

print("Random number:", random_num)
print("Integer number:", integer_num)
print("Hashed value:", hashed_value)
print("Time taken for hashing: {:.6f} seconds".format(execution_time))
