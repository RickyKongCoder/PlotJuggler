
include_directories( ./ ../  ../../include  ../../common ${Qt5SerialPort_INCLUDE_DIRS})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SERIALPORT)

find_package(Qt5 REQUIRED COMPONENTS SerialPort )

#QT5_WRAP_UI ( UI_SRC  ../common/selectlistdialog.ui  )


SET( SRC
    datastreamserial.cpp
    ../../include/PlotJuggler/datastreamer_base.h
    )

add_library(DataStreamSerialPort SHARED ${SRC} ${UI_SRC}  )
target_link_libraries(DataStreamSerialPort  ${Qt5Widgets_LIBRARIES} Qt5::SerialPort Qt5::Xml)


