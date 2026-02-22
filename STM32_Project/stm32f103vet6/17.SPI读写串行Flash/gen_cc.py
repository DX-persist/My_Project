import sys
import json
import subprocess
import os
import re

def main():
    print("Step 1: Cleaning previous build...")
    subprocess.run(['make', 'clean'], check=False)
    
    print("Step 2: Compiling firmware...")
    cmd = ['make', '-B', 'firmware']
    try:
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE, 
            stderr=subprocess.STDOUT, 
            text=True
        )
        lines = result.stdout.splitlines()
    except Exception as e:
        print(f"Error running make: {e}")
        return

    entries = []
    
    # 【关键修改】优先使用 PWD 环境变量获取逻辑路径（保留软链接结构）
    # 如果获取失败，再回退到 os.getcwd()
    cwd = os.getenv('PWD') or os.getcwd()

    gcc_pattern = re.compile(r'arm-none-eabi-gcc\s+.*?-c\s+([^\s]+)')

    for line in lines:
        line = line.strip()
        if 'arm-none-eabi-gcc' in line and ' -c ' in line:
            match = gcc_pattern.search(line)
            if match:
                source_file = match.group(1)
                
                # 【关键修改】手动拼接路径，而不使用 os.path.abspath (因为它会解析软链接)
                # 假设 source_file 是相对路径 (如 User/main.c)
                if not os.path.isabs(source_file):
                    file_path = os.path.join(cwd, source_file)
                else:
                    file_path = source_file # 已经是绝对路径就不动了

                entries.append({
                    "directory": cwd,
                    "command": line, 
                    "file": file_path 
                })

    if entries:
        with open('compile_commands.json', 'w') as f:
            json.dump(entries, f, indent=4)
        print(f"\n[OK] Updated compile_commands.json with {len(entries)} commands.")
        print(f"Path format: {entries[0]['file']}") # 打印一条看看路径格式对不对
    else:
        print("\n[Warning] No compilation commands captured!")

if __name__ == '__main__':
    main()