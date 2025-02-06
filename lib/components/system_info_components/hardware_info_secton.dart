import 'package:flutter/widgets.dart';
import 'info_row.dart';

class HardwareInfoSection extends StatelessWidget {
  const HardwareInfoSection({super.key});

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: const [
        InfoRow(
          iconPath: 'assets/icons/pc.svg',
          title: "CPU",
          details: ["AMD Ryzen 5 800G", "8 Cores"],
        ),
        SizedBox(height: 15),
        InfoRow(
          iconPath: 'assets/icons/pc.svg',
          title: "GPU",
          details: ['GeForce RTX 5090', 'GB202', '2017 MHz'],
        ),
      ],
    );
  }
}
