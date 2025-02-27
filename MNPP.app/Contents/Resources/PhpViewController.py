#
#   PhpViewController.py
#
#   Created by Jair Gaxiola on 23/04/11.
#   Copyright 2011 __MyCompanyName__. All rights reserved.
#

from Foundation import *
from AppKit import *
import objc
class PhpViewController (NSViewController):
	php53 = objc.IBOutlet()
	php52 = objc.IBOutlet()
	phpVersions = objc.IBOutlet()
	
	@objc.IBAction
	def savePreferences_(self, sender):
		settings = NSUserDefaults.standardUserDefaults()
		
		phpVersion = self.phpVersions.selectedCell().tag()

		if phpVersion == 53:
			settings.setObject_forKey_(1, 'php53')
			settings.setObject_forKey_(0, 'php52')
			self.php52.setState_(NSOffState)
		else:
			settings.setObject_forKey_(1, 'php52')
			settings.setObject_forKey_(0, 'php53')
			self.php52.setState_(NSOnState)