import os

class LocalStorageManager:
    def __init__(self):
        pass

    def save_file_locally(self, file_name, file_content):
        with open(file_name, 'wb') as file:
            file.write(file_content)

    def get_file_locally(self, file_name):
        with open(file_name, 'rb') as file:
            return file.read()

    def delete_file_locally(self, file_name):
        if os.path.exists(file_name):
            os.remove(file_name)

if __name__ == "__main__":

    storage_manager = LocalStorageManager()

    content = storage_manager.get_file_locally("dog_run_173.csv")
    print(content.decode())
    storage_manager.delete_file_locally("dog_run_173.csv")