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
                "System Specifications",
                style: TextStyle(fontWeight: FontWeight.bold, fontSize: 16),
              ),
              const SizedBox(height: 15),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: const [
                  SystemInfoSection(),
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

// import 'package:flutter/material.dart';
// import 'package:flutter/widgets.dart';
// import 'package:flutter_svg/flutter_svg.dart';
// import 'package:system_info/theme/container_color.dart';
// import 'package:system_info/theme/container_radius.dart';
// import 'package:system_info/utils/dynamic_size.dart';

// class SystemInfoContainer extends StatelessWidget {
//   const SystemInfoContainer({super.key});

//   @override
//   Widget build(BuildContext context) {
//     return Container(
//       padding: EdgeInsets.only(left: 15, top: 10, right: 15, bottom: 10),
//       width: dynamicWidth(context) * 0.3,
//       decoration: BoxDecoration(
//           color: ContainerColor.primary, borderRadius: ContainerRadius.primary),
//       child: Column(
//         mainAxisAlignment: MainAxisAlignment.start,
//         children: [
//           Text("System Specifications"),
//           Row(
//             mainAxisAlignment: MainAxisAlignment.spaceAround,
//             crossAxisAlignment: CrossAxisAlignment.start,
//             children: [
//               Column(
//                 crossAxisAlignment: CrossAxisAlignment.start,
//                 children: [
//                   SizedBox(
//                     height: 15,
//                   ),
//                   Row(
//                     children: [
//                       SvgPicture.asset(
//                         'assets/icons/pc.svg',
//                         height: 20,
//                       ),
//                       SizedBox(
//                         width: 10,
//                       ),
//                       Column(
//                         crossAxisAlignment: CrossAxisAlignment.start,
//                         children: [
//                           Text("System Info"),
//                           SizedBox(
//                             height: 4,
//                           ),
//                           Text(
//                             "Mac M1",
//                             style: TextStyle(fontSize: 8),
//                           ),
//                           SizedBox(
//                             height: 2,
//                           ),
//                           Text(
//                             "Mac OS 15",
//                             style: TextStyle(fontSize: 8),
//                           )
//                         ],
//                       )
//                     ],
//                   ),
//                   Row(
//                     children: [
//                       SvgPicture.asset(
//                         'assets/icons/pc.svg',
//                         height: 20,
//                       ),
//                       SizedBox(
//                         width: 10,
//                       ),
//                       Column(
//                         crossAxisAlignment: CrossAxisAlignment.start,
//                         children: [
//                           Text("Storage"),
//                           Text(
//                             "types of storage",
//                             style: TextStyle(fontSize: 8),
//                           ),
//                           SizedBox(
//                             height: 2,
//                           ),
//                           Text(
//                             "capacity",
//                             style: TextStyle(fontSize: 8),
//                           )
//                         ],
//                       )
//                     ],
//                   ),
//                   SizedBox(
//                     height: 15,
//                   ),
//                   Row(
//                     children: [
//                       SvgPicture.asset(
//                         'assets/icons/pc.svg',
//                         height: 20,
//                       ),
//                       SizedBox(
//                         width: 10,
//                       ),
//                       Column(
//                         crossAxisAlignment: CrossAxisAlignment.start,
//                         children: [
//                           Text("RAM"),
//                           Text(
//                             "Type of Ram",
//                             style: TextStyle(fontSize: 8),
//                           ),
//                           SizedBox(
//                             height: 2,
//                           ),
//                           Text(
//                             "Capacity",
//                             style: TextStyle(fontSize: 8),
//                           )
//                         ],
//                       )
//                     ],
//                   )
//                 ],
//               ),
//               SizedBox(
//                 width: 30,
//               ),
//               Column(
//                 crossAxisAlignment: CrossAxisAlignment.start,
//                 children: [
//                   SizedBox(
//                     height: 20,
//                   ),
//                   Row(
//                     children: [
//                       SvgPicture.asset(
//                         'assets/icons/pc.svg',
//                         height: 20,
//                       ),
//                       SizedBox(
//                         width: 10,
//                       ),
//                       Column(
//                         children: [Text("CPU")],
//                       )
//                     ],
//                   ),
//                   SizedBox(
//                     height: 15,
//                   ),
//                   Row(
//                     children: [
//                       SvgPicture.asset(
//                         'assets/icons/pc.svg',
//                         height: 20,
//                       ),
//                       SizedBox(
//                         width: 10,
//                       ),
//                       Column(
//                         children: [Text("GPU")],
//                       )
//                     ],
//                   )
//                 ],
//               )
//             ],
//           )
//         ],
//       ),
//     );
//   }
// }
