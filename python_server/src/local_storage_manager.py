import os

class LocalStorageManager:
    def __init__(self):
        pass

    def save_file(self, file_name, file_content):
        with open(file_name, 'wb') as file:
            file.write(file_content)

