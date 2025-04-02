import os
import shutil
from lxml import etree
import zipfile

def remove_comments_and_blank_lines(input_file, output_file):
    try:
        parser = etree.XMLParser(remove_comments=True)
        tree = etree.parse(input_file, parser)
        with open(output_file, 'wb') as f:
            xml_content = etree.tostring(tree, encoding='UTF-8', xml_declaration=True, pretty_print=False)
            non_blank_lines = [line for line in xml_content.decode('utf-8').splitlines() if line.strip()]
            f.write('\n'.join(non_blank_lines).encode('utf-8'))
    except Exception as e:
        print(f"Error processing {input_file}: {e}")

def process_xml_files(input_directory, output_directory):
    if os.path.exists(output_directory):
        shutil.rmtree(output_directory)
    os.makedirs(output_directory)
    for root, dirs, files in os.walk(input_directory):
        for file in files:
            if file.endswith('.xml'):
                input_file_path = os.path.join(root, file)
                relative_path = os.path.relpath(input_file_path, input_directory)
                output_file_path = os.path.join(output_directory, relative_path)
                output_file_dir = os.path.dirname(output_file_path)
                if not os.path.exists(output_file_dir):
                    os.makedirs(output_file_dir)
                remove_comments_and_blank_lines(input_file_path, output_file_path)

def zip_xml_files(output_directory, zip_file_path):
    with zipfile.ZipFile(zip_file_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(output_directory):
            for file in files:
                if file.endswith('.xml'):
                    file_path = os.path.join(root, file)
                    relative_path = os.path.relpath(file_path, output_directory)
                    zipf.write(file_path, relative_path)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 4:
        print("Usage: python remove_xml_comments.py <input_directory> <output_directory> <zip_file_path>")
        sys.exit(1)
    input_directory = sys.argv[1]
    output_directory = sys.argv[2]
    zip_file_path = sys.argv[3]
    process_xml_files(input_directory, output_directory)
    zip_xml_files(output_directory, zip_file_path)
    # 可选：删除临时目录
    shutil.rmtree(output_directory)

