import functools
import os

from PyQt5.QtCore import QDir
from PyQt5.QtWidgets import QMessageBox


class ProjectService(object):
    SERVICE_NAME = "project_service"

    def __init__(self, service_manager):
        self.name = None
        self.project_dir = None
        self._core_dir = None

        self.publish = functools.partial(service_manager.publish, self.SERVICE_NAME, "event")

    def close(self):
        pass

    def api_get_project_dir(self):
        return self.project_dir

    def get_source_dir(self):
        return self.source_dir

    @property
    def source_dir(self):
        return os.path.join(self.project_dir, "src")

    @property
    def build_dir(self):
        return os.path.join(self.project_dir, "build")

    @property
    def core_dir(self):
        return self._core_dir

    @property
    def is_project_opened(self):
        return self.project_dir is not None

    @staticmethod
    def validate_project(project_dir):
        # TODO: reomove qt

        selected_dir = QDir(project_dir)

        if not selected_dir.exists():
            QMessageBox.critical(None, 'Dir does not  exist', 'Dir does not  exist',
                                 QMessageBox.Yes, QMessageBox.Yes)
            return False

        if not selected_dir.exists('src/global.config'):
            QMessageBox.critical(None, 'Project validation error', 'Project dir does not contain src/global.config',
                                 QMessageBox.Yes, QMessageBox.Yes)
            return False

        return True

    def run(self):
        pass

    def open_project(self, name, project_dir, core_dir):
        self._core_dir = core_dir

        if project_dir == '':
            return False

        if name == '':
            return False

        if not self.validate_project(project_dir):
            return False

        if self.is_project_opened:
            self.close_project()

        self.project_dir = project_dir
        self.name = name

        self.publish(dict(msg_type="project_opened", project_dir=project_dir, name=name))

        return True

    def close_project(self):
        self.project_dir = None

        self.publish(dict(msg_type="project_closed"))

    def api_open_project(self, name, project_dir, core_dir):
        return self.open_project(name, project_dir, core_dir)

    def api_close_project(self):
        return self.close_project()
