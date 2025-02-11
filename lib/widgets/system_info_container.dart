import 'package:flutter/material.dart';
import 'package:system_info/theme/container_color.dart';
import 'package:system_info/theme/container_radius.dart';
import 'package:system_info/utils/dynamic_size.dart';
import '../components/system_info_components/hardware_info_secton.dart';
import '../components/system_info_components/system_info_section.dart';

class SystemInfoContainer extends StatelessWidget {
  const SystemInfoContainer({super.key});

  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
        double widthFactor = 0.493;

        return Container(
          padding: const EdgeInsets.symmetric(vertical: 20, horizontal: 1),
          width: dynamicWidth(context) * widthFactor,
          decoration: BoxDecoration(
            color: ContainerColor.primary,
            borderRadius: ContainerRadius.primary,
          ),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              const Text(
                "System info",
                style: TextStyle(fontWeight: FontWeight.bold, fontSize: 16),
              ),
              const SizedBox(height: 15),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: const [
                  SystemInfoSection(),
                  HardwareInfoSection(),
                  HardwareInfoSection(),
                ],
              ),
            ],
          ),
        );
      },
    );
  }
}
