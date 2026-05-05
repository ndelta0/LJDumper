function Component() {
}

Component.prototype.createOperations = function() {
    component.createOperations();

    if (systemInfo.productType !== "windows") {
        return;
    }

    var labJackUninstaller = "C:/Program Files (x86)/LabJack/Uninstall LabJack Full.exe";
    var labJackAlreadyInstalled = installer.fileExists(labJackUninstaller);

    if (labJackAlreadyInstalled) {
        return;
    }
    component.addElevatedOperation(
        "Execute",
        "@TargetDir@/LabJackBasic_2025-02-12.exe",
        "/S",
        "UNDOEXECUTE",
        "C:/Program Files (x86)/LabJack/Uninstall LabJack Full.exe",
        "/S"
    );
}