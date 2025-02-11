import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../../provider/system_info_provider.dart';
import 'info_row.dart';

class SystemInfoSection extends StatelessWidget {
  const SystemInfoSection({super.key});

  @override
  Widget build(BuildContext context) {
    final systemInfo = Provider.of<SystemInfoProvider>(context).systemInfo;

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        InfoRow(
          iconPath: 'assets/icons/pc.svg',
          title: "System Model",
          details: systemInfo["System Info"] ?? [],
        ),
        const SizedBox(height: 10),
        InfoRow(
          iconPath: 'assets/icons/pc.svg',
          title: "Storage",
          details: systemInfo["Storage"] ?? [],
        ),
        const SizedBox(height: 10),
        InfoRow(
          iconPath: 'assets/icons/pc.svg',
          title: "RAM",
          details: systemInfo["RAM"] ?? [],
        ),
      ],
    );
  }
}

// import 'package:flutter/widgets.dart';
// import 'info_row.dart';

// class SystemInfoSection extends StatelessWidget {
//   const SystemInfoSection({super.key});

//   @override
//   Widget build(BuildContext context) {
//     return Column(
//       crossAxisAlignment: CrossAxisAlignment.start,
//       children: const [
//         InfoRow(
//             iconPath: 'assets/icons/pc.svg',
//             title: "System Info",
//             details: ["Mac M1", "Mac OS 15"]),
//         SizedBox(height: 10),
//         InfoRow(
//             iconPath: 'assets/icons/pc.svg',
//             title: "Storage",
//             details: ["SSD", "512GB"]),
//         SizedBox(height: 10),
//         InfoRow(
//             iconPath: 'assets/icons/pc.svg',
//             title: "RAM",
//             details: ["DDR4", "16GB"]),
//       ],
//     );
//   }
// }
