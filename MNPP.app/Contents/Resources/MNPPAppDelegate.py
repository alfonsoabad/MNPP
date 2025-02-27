#
#  MNPPAppDelegate.py
#  MNPP
#
#  Created by Jair Gaxiola on 02/04/11.
#  Copyright __MyCompanyName__ 2011. All rights reserved.
#

from Foundation import *
from AppKit import *
from MNPPController import MNPPController

class MNPPAppDelegate(NSObject):
    statusMenu = objc.IBOutlet()
    startButton = objc.IBOutlet()
    stopButton = objc.IBOutlet()
    startNginx = objc.IBOutlet()
    stopNginx = objc.IBOutlet()
    startMySQL = objc.IBOutlet()
    stopMySQL = objc.IBOutlet()
    startPHP = objc.IBOutlet()
    stopPHP = objc.IBOutlet()
    preferences = objc.IBOutlet()
    step = objc.IBOutlet()

	
    def applicationDidFinishLaunching_(self, sender):
        NSLog("Application did finish launching.")

        self.mnppController = MNPPController.alloc().init()
        #self.mnppController.showWindow_(self)
        self.mnppController.checkSettings()

    #def awakeFromNib(self):		
        statusItem = NSStatusBar.systemStatusBar().statusItemWithLength_(NSVariableStatusItemLength).retain()
        statusItem.setMenu_(self.statusMenu)
        statusItem.setHighlightMode_(YES)
        statusItem.setToolTip_("MNPP: Mac + Nginx + Percona + PHP/Python")
        
        font = NSFontManager.sharedFontManager().convertFont_toHaveTrait_(NSFont.menuBarFontOfSize_(11), NSBoldFontMask)
        #first row has a high line height to keep the icon vertical centered
        parStyleFirstRow = NSParagraphStyle.defaultParagraphStyle().mutableCopy()
        parStyleFirstRow.setAlignment_(NSCenterTextAlignment)
        parStyleFirstRow.setMinimumLineHeight_(15)
        #second row has low line height to keep the icon compact
        parStyleSecondRow = NSParagraphStyle.defaultParagraphStyle().mutableCopy()
        parStyleSecondRow.setParagraphStyle_(parStyleFirstRow)
        parStyleSecondRow.setMaximumLineHeight_(10)
        #the char \u2009 is a thin space
        title = NSMutableAttributedString.alloc().initWithString_attributes_(u"MN\nP\u2009P", {NSFontAttributeName: font})
        title.addAttribute_value_range_(NSParagraphStyleAttributeName, parStyleFirstRow, NSMakeRange(0,3))
        title.addAttribute_value_range_(NSParagraphStyleAttributeName, parStyleSecondRow, NSMakeRange(3,3))
        statusItem.setAttributedTitle_(title)

    @objc.IBAction
    def showPreferencesWindow_(self, sender):
		self.mnppController.showPreferencesWindow_(self)
		
    @objc.IBAction
    def startServers_(self,sender):
		self.changeStatusStartMenuALL()
		self.startNginx.setHidden_(YES)
		self.stopNginx.setHidden_(NO)
		self.startMySQL.setHidden_(YES)
		self.stopMySQL.setHidden_(NO)
		self.startPHP.setHidden_(YES)
		self.stopPHP.setHidden_(NO)
		
		self.mnppController.startServers_(self)

    @objc.IBAction
    def stopServers_(self,sender):
		self.changeStatusStopMenuALL()
		self.startNginx.setHidden_(NO)
		self.stopNginx.setHidden_(YES)
		self.startMySQL.setHidden_(NO)
		self.stopMySQL.setHidden_(YES)
		self.startPHP.setHidden_(NO)
		self.stopPHP.setHidden_(YES)

		self.mnppController.stopServers_(self)

    @objc.IBAction
    def startNginx_(self,sender):
		self.changeStatusStartMenuALL()
		self.startNginx.setHidden_(YES)
		self.stopNginx.setHidden_(NO)
		self.mnppController.startNginx_(self)

    @objc.IBAction
    def stopNginx_(self, sender):
		self.changeStatusStopMenuALL()
		self.stopNginx.setHidden_(YES)
		self.startNginx.setHidden_(NO)
		self.mnppController.stopNginx_(self)
		
    @objc.IBAction
    def startMySQL_(self, sender):
		self.changeStatusStartMenuALL()
		self.startMySQL.setHidden_(YES)
		self.stopMySQL.setHidden_(NO)
		self.mnppController.startMySQL_(self)

    @objc.IBAction
    def stopMySQL_(self, sender):
		self.changeStatusStopMenuALL()
		self.startMySQL.setHidden_(YES)
		self.stopMySQL.setHidden_(NO)
		self.mnppController.stopMySQL_(self)

    @objc.IBAction
    def startPHP_(self, sender):
		self.changeStatusStartMenuALL()
		self.startPHP.setHidden_(YES)
		self.stopPHP.setHidden_(NO)
		self.mnppController.startPHP_(self)

    @objc.IBAction
    def stopPHP_(self, sender):
		self.changeStatusStopMenuALL()
		self.stopPHP.setHidden_(YES)
		self.startPHP.setHidden_(NO)
		self.mnppController.stopPHP_(self)

    @objc.IBAction
    def exit_(self, sender):
		self.mnppController.exit_(self)

    def changeStatusStartMenuALL(self):
		self.startButton.setHidden_(YES)
		self.stopButton.setHidden_(NO)
	
    def changeStatusStopMenuALL(self):
		self.startButton.setHidden_(NO)
		self.stopButton.setHidden_(YES)
