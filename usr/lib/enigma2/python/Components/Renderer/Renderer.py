from Components.GUIComponent import GUIComponent
from Components.Element import Element

class Renderer(GUIComponent, Element):
	CHANGED_PULSATE = 5   # widget should pulsate

	def __init__(self):
		Element.__init__(self)
		GUIComponent.__init__(self)

	def onShow(self):
		self.suspended = False

	def onHide(self):
		self.suspended = True

	def canPulsate(self):
		return False
