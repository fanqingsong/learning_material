#import os
import  time
from selenium import webdriver
from selenium.webdriver.common.keys import Keys

# test case
def testbrowser(driver):
    driver.get("http://www.baidu.com")
    driver.find_element_by_id("kw").click()
    driver.find_element_by_id("kw").clear()
    driver.find_element_by_id("kw").send_keys("vx")
    driver.find_element_by_id("su").click()
    driver.implicitly_wait(30)
    time.sleep(3)
    driver.close()
    driver.quit()
    return None

driverfirefox = webdriver.Firefox()
testbrowser(driverfirefox)

driverie = webdriver.Ie()
testbrowser(driverie)

driverchrome = webdriver.Chrome()
testbrowser(driverchrome)