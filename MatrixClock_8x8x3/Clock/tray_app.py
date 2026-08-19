import pystray
from pystray import MenuItem as item

class TrayApp:
    def __init__(self, image_link, image_unlink, on_refresh, on_exit):
        self.image_link = image_link
        self.image_unlink = image_unlink
        self.icon = pystray.Icon(
            name="Clock",
            title="N.A.",
            icon=self.image_unlink,
            menu=pystray.Menu(
                item("Refresh", on_refresh),
                item("Exit", on_exit)
            )
        )

    def run(self):
        self.icon.run()

    def stop(self):
        self.icon.stop()

    def set_connected(self, port_name):
        self.icon.icon = self.image_link
        self.icon.title = port_name if port_name else "Connected"

    def set_disconnected(self):
        self.icon.icon = self.image_unlink
        self.icon.title = "N.A."
