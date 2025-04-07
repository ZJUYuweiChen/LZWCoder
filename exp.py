import os
import shutil
import subprocess
import time
import matplotlib.pyplot as plt
from tqdm import tqdm

EXP_DIR = "exp"

def format_size(size):
    units = ['B', 'KB', 'MB', 'GB', 'TB']
    unit_index = 0

    while size >= 1024 and unit_index < len(units) - 1:
        size /= 1024
        unit_index += 1

    return f"{int(size)}{units[unit_index]}"

def clear():
    # 清理旧文件
    print("Clearing...")
    for dir in tqdm(os.listdir(EXP_DIR)):
        if os.path.isdir(dir):
            shutil.rmtree(os.path.join(EXP_DIR, dir))

def test(exp_file):
    min_size = 1024              # 初始大小1KB 
    max_size = 64 * 1024 * 1024  # 最大大小64MB
    sizes = []
    filepaths = {}

    size = min_size
    while size <= max_size:
        sizes.append(size)
        size *= 2

    # 生成文本文件
    print("Generating...")
    with open(os.path.join(EXP_DIR, exp_file), 'rb') as f:
        data = f.read(max_size)
    for size in tqdm(sizes):
        subs = []
        for i in range(max_size // size):
            filedir = os.path.join(EXP_DIR, f"{exp_file}_{format_size(size)}")
            filename = f"{exp_file}_{format_size(size)}_{i}.txt"
            filepath = os.path.join(filedir, filename)
            subs.append(filepath)
            if not os.path.exists(filedir):
                os.makedirs(filedir)

            part_start = i * size
            part_end = (i + 1) * size
            data_part = data[part_start:part_end]
            
            with open(filepath, 'wb') as f:
                f.write(data_part)
        filepaths[size] = subs

    # 运行压缩脚本
    print("Compressing...")
    try:
        for size, subs in tqdm(filepaths.items()):
            for filepath in subs:
                start_time = time.time()
                subprocess.run(['./lzwcoder', filepath], check=True)
                end_time = time.time()
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        return

    # 收集压缩率数据
    print("Calculating...")
    compression_rates = {}
    for size, subs in tqdm(filepaths.items()):
        rates = []

        for txt_filepath in subs:
            root, ext = os.path.splitext(txt_filepath)
            lzw_filepath = root + ".lzw"

            if not os.path.exists(lzw_filepath):
                print(f"Warning: File {lzw_filepath} not found")
                continue

            txt_size = os.path.getsize(txt_filepath)
            lzw_size = os.path.getsize(lzw_filepath)
            rates.append(lzw_size / txt_size)

        if rates:
            avg_rate = sum(rates) / len(rates)
            compression_rates[size] = avg_rate

    # 输出压缩速度
    print(f"Compression Speed: {max_size / (1024 * (end_time - start_time))} KB/s")

    # 准备图表数据
    if not compression_rates:
        print("Error: No valid rates")
        return

    x = list(compression_rates.keys())
    y = list(compression_rates.values())

    # 生成x轴标签
    x_ticks = x
    x_labels = []
    for x_tick in x_ticks:
        if x_tick < 1024 * 1024:
            x_labels.append(f"{x_tick//1024}KB")
        else:
            x_labels.append(f"{x_tick//(1024*1024)}MB")

    # 绘制图表
    plt.figure(figsize=(12, 6))
    plt.plot(x, y, marker='o', linestyle='-', linewidth=2, markersize=8)
    plt.xscale('log')
    plt.xlabel('file size', fontsize=12)
    plt.ylabel('compression rate', fontsize=12)
    plt.title(f"Compression Rate of {exp_file}", fontsize=14, pad=20)
    
    plt.xticks(x_ticks, x_labels, rotation=45)
    plt.grid(True, which="both", ls="--", alpha=0.7)
    
    # 调整布局
    plt.tight_layout()
    plt.show()