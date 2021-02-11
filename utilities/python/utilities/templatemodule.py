##
#  @file    templatemodule.py
#  @ingroup template_program
#  @brief   Implementation of the TemplateClass.
#  @author
#  
#  This file can be used as a start point for a new development
#
#  @version 1.0 first released version
# 

 
##
#  @ingroup template_program
#  @brief   A template class doing more or less nothing
#  @author
#  
#  This class can be used as a start point to develop a new Python class.
# 
class TemplateClass:
    
    ##
    #  @brief Initialize class attributes.
    #
    #  @param progParam program parameters read from a job order file.
    #
    def __init__(self, progParam):
        ## Program parameters
        self.progParam = progParam
    
    ##
    #  @brief Print a welcome message.
    #
    def printWelcomeMessage(self):
        print("Welcome!")